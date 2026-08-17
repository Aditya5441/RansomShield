/**
 * @file    bm_adc.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef BM_ADC_H
#define BM_ADC_H

#include "stm32wbxx.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   TODO: describe what bm_adc1_init() does
 */
void bm_adc1_init(void);

/**
 * @brief   TODO: describe what bm_adc_read_channel() does
 * @param   channel  TODO: describe parameter
 * @param   value  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_adc_read_channel(uint32_t channel, uint16_t *value, uint32_t timeout_ms);

/**
 * @brief   TODO: describe what bm_adc1_irq_handler() does
 */
void bm_adc1_irq_handler(void);

#endif
