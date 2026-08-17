/**
 * @file    comm_encryption.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_SECURITY_COMM_ENCRYPTION_H_
#define APPLICATION_SECURITY_COMM_ENCRYPTION_H_

#include <stdint.h>
#include "dsp.h"

typedef struct
{
    float voltage;
    float current;
    float power;

    float accel_internal[3];
    float accel_external[3];

    float network;
    float rf;
    float temperature;

    float ai_output;

} SystemData_t;

typedef struct
{
    uint8_t encrypted_payload[64];
    uint8_t iv[16];
    uint8_t tag[16];

} SecurePacket_t;

/**
 * @brief   Encrypts `data` into `packet` using the current ECDH session
 *          key (see session.h). `features` is still scrubbed via
 *          FS_LowLevelProtect() for anti-forensic RAM hygiene, but no
 *          longer used to derive the key (the old per-packet feature-
 *          derived key had no way to be reproduced by the receiver -
 *          see chat/commit history).
 * @retval  1 on success, 0 if no session key is established yet
 *          (SESSION_Handshake() hasn't succeeded) - caller must not
 *          send `packet` in that case, it would be garbage.
 */
int ENC_Encrypt(SystemData_t *data,
                DSP_Features_t *features,
                SecurePacket_t *packet);

#endif
