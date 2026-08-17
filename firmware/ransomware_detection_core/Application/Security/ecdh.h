/**
 * @file    ecdh.h
 * @brief   NIST P-256 ECDH primitives built on the STM32WB55's PKA
 *          (Public Key Accelerator) peripheral. Used to establish a
 *          session AES key with the ESP32 gateway instead of the old
 *          per-packet feature-derived key (which the receiver had no
 *          way to reproduce - see comm_encryption.c).
 *
 * NOTE ON HAL API: this is written against the common
 * stm32wbxx_hal_pka.h shape (HAL_PKA_ECCMul / PKA_ECCMulInTypeDef).
 * Field names have shifted slightly between STM32Cube_FW_WB releases -
 * check your installed SDK's stm32wbxx_hal_pka.h against the field
 * names used in ecdh.c before building, and adjust if your version
 * differs.
 */
#ifndef APPLICATION_SECURITY_ECDH_H_
#define APPLICATION_SECURITY_ECDH_H_

#include <stdint.h>

#define ECDH_PRIV_LEN   32U   /* P-256 scalar */
#define ECDH_PUB_LEN    64U   /* uncompressed point: 32B X || 32B Y */

/**
 * @brief   Generates an ephemeral P-256 keypair: a random private
 *          scalar (via hardware RNG) and the corresponding public
 *          point priv * G.
 */
void ECDH_GenerateKeypair(uint8_t priv[ECDH_PRIV_LEN], uint8_t pub[ECDH_PUB_LEN]);

/**
 * @brief   Computes the ECDH shared secret priv * peer_pub, returning
 *          the X-coordinate only (standard ECDH output - the Y
 *          coordinate is discarded per convention).
 */
void ECDH_ComputeShared(const uint8_t priv[ECDH_PRIV_LEN],
                         const uint8_t peer_pub[ECDH_PUB_LEN],
                         uint8_t shared_x[32]);

#endif /* APPLICATION_SECURITY_ECDH_H_ */
