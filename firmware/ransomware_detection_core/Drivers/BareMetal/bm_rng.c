/**
 * @file    bm_rng.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm.h"

static volatile bool s_rng_ready;

/**
 * @brief   TODO: describe what bm_rng_init() does
 */
void bm_rng_init(void)
{
    const uint32_t prio_group = 3U;

    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_RNGSEL) | RCC_CCIPR_RNGSEL_1;
    RCC->AHB3ENR |= RCC_AHB3ENR_RNGEN;
    RNG->CR = RNG_CR_CED | RNG_CR_RNGEN | RNG_CR_IE;

    NVIC_SetPriority(RNG_IRQn, NVIC_EncodePriority(prio_group, 13U, 0));
    NVIC_EnableIRQ(RNG_IRQn);
}

/**
 * @brief   TODO: describe what bm_rng_irq_handler() does
 */
void bm_rng_irq_handler(void)
{
    if (RNG->SR & RNG_SR_DRDY) {
        s_rng_ready = true;
    }
    if (RNG->SR & (RNG_SR_CECS | RNG_SR_SECS)) {
        s_rng_ready = false;
    }
}

/**
 * @brief   TODO: describe what bm_rng_read_u32() does
 * @param   value  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_rng_read_u32(uint32_t *value, uint32_t timeout_ms)
{
    if (!value) {
        return false;
    }

    s_rng_ready = false;

    if (RNG->SR & RNG_SR_DRDY) {
        *value = RNG->DR;
        return true;
    }

    uint32_t deadline = bm_tick_get() + timeout_ms;
    while (!s_rng_ready) {
        if (RNG->SR & (RNG_SR_CECS | RNG_SR_SECS)) {
            return false;
        }
        if (bm_tick_get() >= deadline) {
            return false;
        }
    }

    *value = RNG->DR;
    s_rng_ready = false;
    return true;
}
