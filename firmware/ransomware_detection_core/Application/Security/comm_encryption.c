/**
 * @file    comm_encryption.c
 * @brief   Encrypts SystemData_t for transmission to the ESP32 gateway,
 *          using the AES-256 session key established via ECDH
 *          (session.h/session.c) rather than a key derived from local
 *          DSP feature values. The feature-derived scheme this
 *          replaced had no way for the ESP32 to reproduce the same
 *          key (it never received the features or the RNG value used
 *          to mix them), so nothing sent under it was ever actually
 *          decryptable by the receiver.
 */

#include "comm_encryption.h"
#include "feature_protection.h"
#include "session.h"
#include "chaos.h"
#include <stdint.h>
#include <string.h>
#include "bm.h"

int ENC_Encrypt(SystemData_t *data,
                DSP_Features_t *features,
                SecurePacket_t *packet)
{
    uint8_t key[32];

    if (!SESSION_GetKey(key))
    {
        /* No session key yet (handshake hasn't run/succeeded). Still
         * scrub features for RAM hygiene, but do not encrypt/send
         * garbage under an all-zero or stale key. */
        FS_LowLevelProtect(features);
        return 0;
    }

    /* Scrub features AFTER they've served their purpose elsewhere in
     * the pipeline (AI input, diagnostics, etc.) - key material no
     * longer comes from here, but the RAM-scrub is still worth doing
     * on its own merits (anti-forensic hygiene). */
    FS_LowLevelProtect(features);

    for (int i = 0; i < 4; i++) {
        bm_rng_read_u32((uint32_t *)&packet->iv[i * 4], 100);
    }

    uint8_t plain[64];
    uint32_t size = sizeof(plain);

    memset(plain, 0, sizeof(plain));
    memcpy(plain, data, sizeof(SystemData_t));

    for (uint32_t i = 0; i < size; i += 16U) {
        bm_aes_cbc_encrypt(key, packet->iv,
                           plain + i,
                           packet->encrypted_payload + i,
                           16U, 100);
    }

    /* Authentication tag: CBC-MAC of the plaintext under the same key
     * with a fixed (zero) IV, independent of the confidentiality IV
     * above. Only 16 bytes (one AES block), matching
     * SecurePacket_t::tag[16].
     *
     * NOTE: a raw CBC-MAC is only secure for fixed-length messages -
     * that holds here since `plain` is always exactly 64 bytes, but if
     * this is ever reused for variable-length payloads, switch to a
     * proper MAC (e.g. CMAC/HMAC) instead of CBC-MAC.
     */
    static const uint8_t zero_iv[16] = {0};
    uint8_t mac_buf[64];
    for (uint32_t i = 0; i < size; i += 16U) {
        bm_aes_cbc_encrypt(key, (i == 0U) ? zero_iv : (mac_buf + i - 16U),
                           plain + i, mac_buf + i, 16U, 100);
    }
    memcpy(packet->tag, mac_buf + (size - 16U), 16U);

    /* Scrub the session key copy from this stack frame - the master
     * copy lives in session.c's static buffer for reuse next packet,
     * this is just the local working copy. */
    memset(key, 0, sizeof(key));

    return 1;
}
