/**
 * @file    bm_tim.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef BM_TIM_H
#define BM_TIM_H

#include <stdint.h>

/**
 * @brief   TODO: describe what bm_tick_get() does
 * @retval  TODO: describe return value
 */
uint32_t bm_tick_get(void);

/**
 * @brief   TODO: describe what bm_tick_inc() does
 */
void bm_tick_inc(void);

/**
 * @brief   TODO: describe what bm_tim1_tick_init() does
 * @param   tick_priority  TODO: describe parameter
 */
void bm_tim1_tick_init(uint32_t tick_priority);

/**
 * @brief   TODO: describe what bm_tim1_irq_handler() does
 */
void bm_tim1_irq_handler(void);

#endif
