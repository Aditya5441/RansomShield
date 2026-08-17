/**
 * @file    bm_aes.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm.h"
#include <string.h>

static volatile bool s_aes_ccf;

/**
 * @brief   TODO: describe what bm_aes1_init() does
 */
void bm_aes1_init(void)
{
    const uint32_t prio_group = 3U;

    RCC->AHB2ENR |= RCC_AHB2ENR_AES1EN;

    NVIC_SetPriority(AES1_IRQn, NVIC_EncodePriority(prio_group, 10U, 0));
    NVIC_EnableIRQ(AES1_IRQn);
}

/**
 * @brief   TODO: describe what bm_aes1_irq_handler() does
 */
void bm_aes1_irq_handler(void)
{
    if (AES1->SR & AES_SR_CCF) {
        s_aes_ccf = true;
    }
}

/**
 * @brief   TODO: describe what aes_load_key_iv() does
 * @param   key_16  TODO: describe parameter
 * @param   iv_16  TODO: describe parameter
 */
static void aes_load_key_iv(const uint8_t *key_16, const uint8_t *iv_16)
{
    const uint32_t *k = (const uint32_t *)key_16;
    const uint32_t *v = (const uint32_t *)iv_16;

    AES1->KEYR0 = k[0];
    AES1->KEYR1 = k[1];
    AES1->KEYR2 = k[2];
    AES1->KEYR3 = k[3];
    AES1->IVR0  = v[0];
    AES1->IVR1  = v[1];
    AES1->IVR2  = v[2];
    AES1->IVR3  = v[3];
}

/**
 * @brief   TODO: describe what aes_wait_ccf() does
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static bool aes_wait_ccf(uint32_t timeout_ms)
{
    uint32_t deadline = bm_tick_get() + timeout_ms;
    while (!s_aes_ccf) {
        if (bm_tick_get() >= deadline) {
            return false;
        }
    }
    s_aes_ccf = false;
    return true;
}

/**
 * @brief   AES-128 CBC encrypt, one block at a time, via the hardware
 *          AES1 peripheral.
 * @param   key_16      16-byte key. Must be non-NULL - see note below.
 * @param   iv_16       16-byte IV. Must be non-NULL - see note below.
 * @param   in           Plaintext, must be a multiple of 16 bytes.
 * @param   out          Ciphertext output buffer, same length as `in`.
 * @param   len_bytes    Length of `in`/`out`, must be a nonzero multiple
 *                        of 16.
 * @param   timeout_ms   Per-block timeout waiting for the AES1 "computation
 *                        complete" flag.
 * @retval  true on success, false on a bad argument, timeout, or missing
 *          key/IV.
 * @note    `key_16`/`iv_16` are REQUIRED (non-NULL). This used to fall
 *          back to a hardcoded key and an all-zero IV when either was
 *          NULL - a silent landmine: any future caller that forgot to
 *          pass a real key/IV would get "successful" encryption under a
 *          key baked into the firmware image, instead of an error. It
 *          now fails closed instead.
 */
bool bm_aes_cbc_encrypt(const uint8_t *key_16, const uint8_t *iv_16,
                        const uint8_t *in, uint8_t *out, uint32_t len_bytes,
                        uint32_t timeout_ms)
{
    if (!key_16 || !iv_16 || !in || !out ||
        (len_bytes == 0U) || (len_bytes % 16U) != 0U) {
        return false;
    }

    aes_load_key_iv(key_16, iv_16);

    AES1->CR = AES_CR_EN | AES_CR_DATATYPE_1 | AES_CR_CHMOD_0 | AES_CR_CCFIE;

    for (uint32_t offset = 0; offset < len_bytes; offset += 16U) {
        const uint32_t *win = (const uint32_t *)(in + offset);
        for (int i = 0; i < 4; i++) {
            AES1->DINR = win[i];
        }

        if (!aes_wait_ccf(timeout_ms)) {
            AES1->CR &= ~(AES_CR_EN | AES_CR_CCFIE);
            return false;
        }

        uint32_t *wout = (uint32_t *)(out + offset);
        for (int i = 0; i < 4; i++) {
            wout[i] = AES1->DOUTR;
        }
        AES1->CR |= AES_CR_CCFC;
    }

    AES1->CR &= ~(AES_CR_EN | AES_CR_CCFIE);
    return true;
}
