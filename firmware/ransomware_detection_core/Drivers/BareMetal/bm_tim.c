/**
 * @file    bm_tim.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm.h"

static volatile uint32_t s_uw_tick;

/**
 * @brief   TODO: describe what bm_tick_get() does
 * @retval  TODO: describe return value
 */
uint32_t bm_tick_get(void)
{
    return s_uw_tick;
}

/**
 * @brief   TODO: describe what bm_tick_inc() does
 */
void bm_tick_inc(void)
{
    s_uw_tick++;
}

/**
 * @brief   TODO: describe what bm_tim1_tick_init() does
 * @param   tick_priority  TODO: describe parameter
 */
void bm_tim1_tick_init(uint32_t tick_priority)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    TIM1->CR1 = 0;
    TIM1->PSC = (SystemCoreClock / 1000000U) - 1U;
    TIM1->ARR = 1000U - 1U;
    TIM1->EGR = TIM_EGR_UG;
    TIM1->SR  = 0;
    TIM1->DIER = TIM_DIER_UIE;
    TIM1->CR1  = TIM_CR1_CEN;

    NVIC_SetPriority(TIM1_UP_TIM16_IRQn, tick_priority);
    NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
}

/**
 * @brief   TODO: describe what bm_tim1_irq_handler() does
 */
void bm_tim1_irq_handler(void)
{
    if (TIM1->SR & TIM_SR_UIF) {
        TIM1->SR = ~TIM_SR_UIF;
        bm_tick_inc();
    }
}
