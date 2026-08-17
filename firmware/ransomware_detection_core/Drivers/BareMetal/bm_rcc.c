/**
 * @file    bm_rcc.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm.h"

extern uint32_t SystemCoreClock;

/**
 * @brief   Brings up the board's clock tree: MSI (SYSCLK/LPUART source,
 *          4 MHz), HSI16 (SMPS step-down regulator source), and LSI
 *          (RNG source, via RCC_CCIPR_RNGSEL = 0b10). HSE is
 *          intentionally NOT enabled here: nothing on this board
 *          selects it as a clock source (SYSCLK stays on MSI,
 *          LPUART1SEL = SYSCLK, RNGSEL = LSI) - turning it on would
 *          just burn power waiting on an oscillator nothing uses.
 */
void bm_rcc_init(void)
{
    RCC->CR |= RCC_CR_MSION;
    while (!(RCC->CR & RCC_CR_MSIRDY));

    /* HSI16: required by the SMPS step-down regulator (see
     * RCC_CR_HSIASFS / RCC_SMPSCR_SMPSSEL below), not by SYSCLK. */
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    /* LSI: required by the hardware RNG (RCC_CCIPR_RNGSEL = 0b10
     * selects LSI as its clock, set in bm_rng_init()). */
    RCC->CSR |= RCC_CSR_LSI1ON;
    while (!(RCC->CSR & RCC_CSR_LSI1RDY));

    RCC->CR = (RCC->CR & ~RCC_CR_MSIRANGE) | RCC_CR_MSIRANGE_6;
    while (!(RCC->CR & RCC_CR_MSIRDY));

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | (0U << RCC_CFGR_SW_Pos);
    while ((RCC->CFGR & RCC_CFGR_SWS) != 0U);

    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_0;

    RCC->CR |= RCC_CR_HSIASFS;
    RCC->SMPSCR = (RCC->SMPSCR & ~RCC_SMPSCR_SMPSSEL) | RCC_SMPSCR_SMPSSEL_0;

    SystemCoreClock = 4000000U;
}

/**
 * @brief   TODO: describe what bm_delay_ms() does
 * @param   ms  TODO: describe parameter
 */
void bm_delay_ms(uint32_t ms)
{
    uint32_t start = bm_tick_get();
    while ((bm_tick_get() - start) < ms) {
        __NOP();
    }
}
