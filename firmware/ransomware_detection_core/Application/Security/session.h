/**
 * @file    session.h
 * @brief   Establishes and holds the AES-256 session key shared with
 *          the ESP32 gateway, derived via ECDH. Replaces the old
 *          per-packet feature-derived key in comm_encryption.c, which
 *          the receiver had no way to reproduce.
 */
#ifndef APPLICATION_SECURITY_SESSION_H_
#define APPLICATION_SECURITY_SESSION_H_

#include <stdint.h>

/**
 * @brief   Runs the ECDH handshake over LPUART: sends our ephemeral
 *          public key, waits for the ESP32's, computes the shared
 *          secret, and derives a 32-byte AES-256 session key from it.
 *          Blocking; call once at boot before starting the Comm task.
 * @retval  1 on success, 0 on timeout/failure (no peer response, or a
 *          malformed public key received).
 */
int SESSION_Handshake(void);

/**
 * @brief   Copies the current 32-byte session key into `out`.
 *          SESSION_Handshake() must have succeeded first; if it
 *          hasn't, this returns 0 and does not touch `out` (the
 *          caller should not silently encrypt with a zero/stale key).
 */
int SESSION_GetKey(uint8_t out[32]);

#endif /* APPLICATION_SECURITY_SESSION_H_ */
