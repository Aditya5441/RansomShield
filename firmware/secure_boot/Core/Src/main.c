/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body for secure bootloader - BAREMETAL build
  ******************************************************************************
  * Baremetal port: this file no longer uses ST's HAL. Every peripheral is
  * configured by writing its registers directly through the CMSIS device
  * header (stm32wb55xx.h -> RCC, GPIOx, USART1, ...). Behaviour matches the
  * original HAL-based SystemClock_Config()/PeriphCommonClock_Config()/
  * MX_GPIO_Init()/MX_USART1_Init():
  *   - MSI range 6 (4 MHz) selected as SYSCLK, HSE and HSI also enabled
  *   - AHB/APB1/APB2 prescalers = /1, Flash latency = 0 WS
  *   - SMPS clock source = HSI
  *   - GPIOA/GPIOC clocks enabled
  *   - USART1 8N1 @115200, TX/RX enabled
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "secure_boot.h"
#include "bootloader.h"

/* Private variables ----------------------------------------------------------*/
volatile uint32_t g_systick_ms = 0;   /* free-running millisecond tick */

/* Private function prototypes -------------------------------------------------*/
static void SysTick_Init(void);
static uint32_t wait_flag_set(volatile uint32_t *reg, uint32_t mask, uint32_t timeout_ms);

/**
  * @brief  The application entry point.
  */
int main(void)
{
    /* SystemInit() (CMSIS) already ran from the reset handler before main().
     * It only sets up FPU/vector-table relocation - no clock changes. */

    /* SysTick first (like HAL_Init()->HAL_InitTick()) so the timeouts used
     * inside SystemClock_Config() have a running millisecond counter. It
     * runs off the reset-default 4 MHz MSI clock until SystemClock_Config()
     * re-derives SystemCoreClock, which is fine for these short timeouts. */
    SysTick_Init();
    SystemClock_Config();
    GPIO_Init();
    USART1_Init();

    /* Run secure bootloader */
    Bootloader_Run();

    /* Should never reach here unless bootloader stays in recovery mode */
    while (1)
    {
    }
}

/**
  * @brief  Busy-wait for (*reg & mask) == mask, with a millisecond timeout.
  * @retval 1 on success, 0 on timeout
  */
static uint32_t wait_flag_set(volatile uint32_t *reg, uint32_t mask, uint32_t timeout_ms)
{
    uint32_t start = g_systick_ms;
    while ((*reg & mask) != mask)
    {
        if ((g_systick_ms - start) > timeout_ms)
        {
            return 0U;
        }
    }
    return 1U;
}

/**
  * @brief  1 ms SysTick, HCLK-referenced. Used only for init timeouts here.
  */
static void SysTick_Init(void)
{
    /* SystemCoreClock is kept up to date by SystemClock_Config()/
     * SystemCoreClockUpdate() (CMSIS, in system_stm32wbxx.c). */
    SysTick_Config(SystemCoreClock / 1000U);
}

void SysTick_Handler(void)
{
    g_systick_ms++;
}

/**
  * @brief  Millisecond delay (replacement for HAL_Delay()).
  */
void Delay_ms(uint32_t ms)
{
    uint32_t start = g_systick_ms;
    while ((g_systick_ms - start) < ms)
    {
    }
}

/**
  * @brief System Clock Configuration (register-level, replaces
  *        HAL's SystemClock_Config()+PeriphCommonClock_Config()).
  */
void SystemClock_Config(void)
{
    /* --- Voltage scaling: range 1 (matches __HAL_PWR_VOLTAGESCALING_CONFIG) --- */
    PWR->CR1 = (PWR->CR1 & ~PWR_CR1_VOS) | PWR_CR1_VOS_0;
    while ((PWR->SR2 & PWR_SR2_VOSF) != 0U)
    {
        /* wait for voltage scaling to apply */
    }

    /* --- Enable HSI, HSE, MSI (matches RCC_OscInitStruct in original) --- */
    RCC->CR |= RCC_CR_HSION;
    if (wait_flag_set(&RCC->CR, RCC_CR_HSIRDY, 5U) == 0U)
    {
        Error_Handler();
    }

    RCC->CR |= RCC_CR_HSEON;
    if (wait_flag_set(&RCC->CR, RCC_CR_HSERDY, 100U) == 0U)
    {
        Error_Handler();
    }

    /* MSI range 6 = 4 MHz (RCC_MSIRANGE_6). Unlike STM32L4, WB55's RCC_CR has
     * no MSIRGSEL bit - CR.MSIRANGE is used directly whenever MSI is running. */
    RCC->CR |= RCC_CR_MSION;
    if (wait_flag_set(&RCC->CR, RCC_CR_MSIRDY, 5U) == 0U)
    {
        Error_Handler();
    }
    RCC->CR = (RCC->CR & ~RCC_CR_MSIRANGE) | RCC_CR_MSIRANGE_6;

    /* --- Flash latency = 0 WS (system clock stays at 4 MHz) --- */
    FLASH->ACR &= ~FLASH_ACR_LATENCY;   /* FLASH_LATENCY_0 */

    /* --- Select MSI as SYSCLK source (SW = 00b = MSI, SWS reads 00b once
     * the switch has taken effect) --- */
    RCC->CFGR &= ~RCC_CFGR_SW;
    {
        uint32_t start = g_systick_ms;
        while ((RCC->CFGR & RCC_CFGR_SWS) != 0U)
        {
            if ((g_systick_ms - start) > 5U)
            {
                Error_Handler();
            }
        }
    }

    /* --- AHB/APB1/APB2/AHB2/AHB4 prescalers = /1 (all *_CLOCKTYPE fields) --- */
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    RCC->EXTCFGR &= ~(RCC_EXTCFGR_C2HPRE | RCC_EXTCFGR_SHDHPRE);

    SystemCoreClockUpdate();

    /* --- Peripheral common clock: SMPS source = HSI, div = range1 --- */
    RCC->SMPSCR = (RCC->SMPSCR & ~(RCC_SMPSCR_SMPSSEL | RCC_SMPSCR_SMPSDIV))
                  | RCC_SMPSCR_SMPSSEL_1 /* HSI */;
}

/**
  * @brief GPIO Initialization Function (enable GPIOA/GPIOC clocks only,
  *        matching the original MX_GPIO_Init()).
  */
void GPIO_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN | RCC_AHB2ENR_GPIOAEN;
    /* Dummy read: RCC clock-enable bits need a couple of AHB clock cycles to
     * propagate before the peripheral can be safely accessed. */
    (void)RCC->AHB2ENR;

    /* USART1 TX = PA9, RX = PA10, AF7, alternate function, very high speed,
     * push-pull, no pull (matches CubeMX's default USART1 pin config). */
    GPIOA->MODER = (GPIOA->MODER & ~((3U << (9U * 2U)) | (3U << (10U * 2U))))
                   | (2U << (9U * 2U)) | (2U << (10U * 2U));      /* AF mode */
    GPIOA->OSPEEDR |= (3U << (9U * 2U)) | (3U << (10U * 2U));     /* very high speed */
    GPIOA->OTYPER  &= ~((1U << 9U) | (1U << 10U));                /* push-pull */
    GPIOA->PUPDR   &= ~((3U << (9U * 2U)) | (3U << (10U * 2U)));  /* no pull */
    GPIOA->AFR[1]  = (GPIOA->AFR[1] & ~((0xFU << ((9U - 8U) * 4U)) | (0xFU << ((10U - 8U) * 4U))))
                     | (7U << ((9U - 8U) * 4U)) | (7U << ((10U - 8U) * 4U));  /* AF7 = USART1 */
}

/**
  * @brief USART1 Initialization Function (115200 8N1, TX+RX, replaces
  *        HAL_USART_Init()+FIFO threshold calls).
  */
void USART1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    (void)RCC->APB2ENR;

    /* USART1 clocked from PCLK2 (default RCC_CCIPR USART1SEL = 00 = PCLK2). */
    USART1->CR1 = 0U;   /* disable + reset before reconfiguring */

    /* Baud rate: BRR = fCK / baud (oversampling by 16, the reset default). */
    USART1->BRR = (uint16_t)((SystemCoreClock + (115200U / 2U)) / 115200U);

    USART1->CR2 = 0U;   /* 1 stop bit, no clock output (async mode) */
    USART1->CR3 = 0U;   /* no hw flow control */

    /* 8 data bits, no parity, TX+RX enable, then USART enable. */
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE;
    USART1->CR1 |= USART_CR1_UE;
}

/**
  * @brief  Blocking single-byte transmit over USART1 (helper for any
  *         diagnostic printing the bootloader stages want to do).
  */
void USART1_SendByte(uint8_t b)
{
    while ((USART1->ISR & USART_ISR_TXE) == 0U)
    {
    }
    USART1->TDR = b;
}

/**
  * @brief  This function is executed in case of error occurrence.
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif /* USE_FULL_ASSERT */
