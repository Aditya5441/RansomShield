/**
 * @file    bm_uart.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm_dma.h"
#include "bm.h"

static volatile bool s_lpuart_tc;

/**
 * @brief   TODO: describe what bm_lpuart1_init() does
 */
void bm_lpuart1_init(void)
{
    const uint32_t prio_group = 3U;

    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_LPUART1SEL) | RCC_CCIPR_LPUART1SEL_0;
    RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    GPIOA->MODER = (GPIOA->MODER & ~((3U << 4) | (3U << 6))) | (2U << 4) | (2U << 6);
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~0xFF0U) | 0x880U;

    LPUART1->CR1 = 0;
    /* LPUART uses a 256x oversampling scheme: BRR = 256 * fclk / baud
     * (NOT the plain fclk/baud division used by regular USARTs). */
    LPUART1->BRR = (uint32_t)(((uint64_t)SystemCoreClock * 256ULL) / 115200ULL);
    LPUART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
    while (!(LPUART1->ISR & USART_ISR_TEACK)) {
    }

    NVIC_SetPriority(LPUART1_IRQn, NVIC_EncodePriority(prio_group, 12U, 0));
    NVIC_EnableIRQ(LPUART1_IRQn);
}

/**
 * @brief   TODO: describe what bm_lpuart1_irq_handler() does
 */
void bm_lpuart1_irq_handler(void)
{
    if (LPUART1->ISR & USART_ISR_TC) {
        LPUART1->ICR = USART_ICR_TCCF;
        s_lpuart_tc = true;
    }
}

/**
 * @brief   TODO: describe what bm_lpuart_transmit() does
 * @param   data  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_lpuart_transmit(const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    if (!data || len == 0U) {
        return false;
    }

    s_lpuart_tc = false;
    LPUART1->CR1 |= USART_CR1_TCIE;

    bm_dma_ch_clear_tc(BM_DMA_CH_LPUART1_TX);
    if (!bm_dma_ch_start(BM_DMA_CH_LPUART1_TX, BM_DMAMUX_REQ_LPUART1_TX,
                         (uint32_t)&LPUART1->TDR, (uint32_t)data, len,
                         false, true, false, 0U)) {
        LPUART1->CR1 &= ~USART_CR1_TCIE;
        return false;
    }

    LPUART1->CR3 |= USART_CR3_DMAT;

    if (!bm_dma_ch_wait_tc(BM_DMA_CH_LPUART1_TX, timeout_ms)) {
        LPUART1->CR3 &= ~USART_CR3_DMAT;
        LPUART1->CR1 &= ~USART_CR1_TCIE;
        return false;
    }

    LPUART1->CR3 &= ~USART_CR3_DMAT;

    uint32_t deadline = bm_tick_get() + timeout_ms;
    while (!s_lpuart_tc) {
        if (bm_tick_get() >= deadline) {
            LPUART1->CR1 &= ~USART_CR1_TCIE;
            return false;
        }
    }
    LPUART1->CR1 &= ~USART_CR1_TCIE;
    return true;
}
