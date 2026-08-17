/**
 * @file    ecdsa_verify.h
 * @brief   NIST P-256 ECDSA signature verification via the STM32WB55
 *          PKA peripheral. Used by SecureBoot_VerifyFirmware() in both
 *          the Secure Boot and Bootloader stages.
 *
 * Signature and public key sizes are the REAL P-256 sizes: a 64-byte
 * public key (32-byte X || 32-byte Y, uncompressed point) and a 64-byte
 * signature (32-byte R || 32-byte S). The earlier draft used pubkey[512]
 * and signature[256], which don't correspond to any real P-256 format -
 * fixed here.
 */
#ifndef APPLICATION_SECURITY_ECDSA_VERIFY_H_
#define APPLICATION_SECURITY_ECDSA_VERIFY_H_

#include <stdint.h>

#define ECDSA_PUBKEY_LEN    64U  /* X (32) || Y (32) */
#define ECDSA_SIGNATURE_LEN 64U  /* R (32) || S (32) */
#define ECDSA_HASH_LEN      32U  /* SHA-256 digest */

/**
 * @retval  1 if the signature is valid for this hash+pubkey, 0 otherwise
 *          (including on any PKA error - fails closed).
 */
int ECDSA_VerifyP256(const uint8_t pubkey[ECDSA_PUBKEY_LEN],
                      const uint8_t hash[ECDSA_HASH_LEN],
                      const uint8_t signature[ECDSA_SIGNATURE_LEN]);

#endif /* APPLICATION_SECURITY_ECDSA_VERIFY_H_ */
