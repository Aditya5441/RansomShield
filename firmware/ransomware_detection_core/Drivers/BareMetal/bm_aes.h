/**
 * @file    bm_aes.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef BM_AES_H
#define BM_AES_H

#include "stm32wbxx.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   TODO: describe what bm_aes1_init() does
 */
void bm_aes1_init(void);

/**
 * @brief   TODO: describe what bm_aes_cbc_encrypt() does
 * @param   key_16  TODO: describe parameter
 * @param   iv_16  TODO: describe parameter
 * @param   in  TODO: describe parameter
 * @param   out  TODO: describe parameter
 * @param   len_bytes  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_aes_cbc_encrypt(const uint8_t *key_16, const uint8_t *iv_16,
                        const uint8_t *in, uint8_t *out, uint32_t len_bytes,
                        uint32_t timeout_ms);

/**
 * @brief   TODO: describe what bm_aes1_irq_handler() does
 */
void bm_aes1_irq_handler(void);

#endif
