/**
 * @file    bm_rng.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef BM_RNG_H
#define BM_RNG_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   TODO: describe what bm_rng_init() does
 */
void bm_rng_init(void);

/**
 * @brief   TODO: describe what bm_rng_read_u32() does
 * @param   value  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_rng_read_u32(uint32_t *value, uint32_t timeout_ms);

/**
 * @brief   TODO: describe what bm_rng_irq_handler() does
 */
void bm_rng_irq_handler(void);

#endif
