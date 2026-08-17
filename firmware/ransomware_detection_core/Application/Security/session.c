/**
 * @file    session.c
 * @brief   See session.h.
 *
 * Wire format (matches the ESP32-side handshake code):
 *   TX: "HELO" (4 bytes) + our 64-byte public key (X || Y)
 *   RX: "HELO" (4 bytes) + peer's 64-byte public key (X || Y)
 * Whichever side reads a well-formed HELO frame within the timeout
 * proceeds; no explicit roles (initiator/responder) are needed since
 * ECDH is symmetric - both sides just need to end up with the same
 * shared secret. If HELO isn't seen within HANDSHAKE_TIMEOUT_MS, the
 * device stays without a session key and the Comm task must not send
 * (see comm_encryption.c / vCommTask - callers must check
 * SESSION_GetKey()'s return value).
 */
#include "session.h"
#include "ecdh.h"
#include "crypto.h"
#include "bm.h"
#include <string.h>

#define HANDSHAKE_TIMEOUT_MS  2000U
#define HELLO_MAGIC_LEN       4U

static uint8_t s_session_key[32];
static uint8_t s_have_key = 0U;

int SESSION_Handshake(void)
{
    uint8_t priv[ECDH_PRIV_LEN];
    uint8_t our_pub[ECDH_PUB_LEN];
    uint8_t peer_pub[ECDH_PUB_LEN];
    uint8_t rx_frame[HELLO_MAGIC_LEN + ECDH_PUB_LEN];
    uint8_t tx_frame[HELLO_MAGIC_LEN + ECDH_PUB_LEN];
    uint8_t shared_x[32];

    s_have_key = 0U;

    ECDH_GenerateKeypair(priv, our_pub);

    memcpy(tx_frame, "HELO", HELLO_MAGIC_LEN);
    memcpy(tx_frame + HELLO_MAGIC_LEN, our_pub, ECDH_PUB_LEN);
    bm_lpuart_transmit(tx_frame, sizeof(tx_frame), 100);

    if (bm_lpuart_receive(rx_frame, sizeof(rx_frame), HANDSHAKE_TIMEOUT_MS) != 0)
    {
        return 0;  // No response from ESP32 within timeout
    }

    if (memcmp(rx_frame, "HELO", HELLO_MAGIC_LEN) != 0)
    {
        return 0;  // Malformed/unexpected frame
    }

    memcpy(peer_pub, rx_frame + HELLO_MAGIC_LEN, ECDH_PUB_LEN);

    ECDH_ComputeShared(priv, peer_pub, shared_x);

    /* Derive the AES-256 key from the raw ECDH output rather than
     * using shared_x directly as the key - a single SHA-256 pass acts
     * as a basic KDF so the key isn't literally the raw curve
     * coordinate. Context string binds the key to this specific
     * protocol/purpose (cheap, standard practice; a real HKDF with a
     * random salt would be stronger but this is a reasonable step up
     * from using the raw shared secret directly). */
    uint8_t kdf_input[32 + 13];
    memcpy(kdf_input, shared_x, 32);
    memcpy(kdf_input + 32, "AEGIS-SESSION", 13);
    CRYPTO_SHA256(kdf_input, sizeof(kdf_input), s_session_key);

    /* Scrub the private scalar and raw shared secret now that the
     * derived key is in place - same "don't leave secrets sitting in
     * RAM longer than needed" principle as FS_LowLevelProtect(). */
    memset(priv, 0, sizeof(priv));
    memset(shared_x, 0, sizeof(shared_x));

    s_have_key = 1U;
    return 1;
}

int SESSION_GetKey(uint8_t out[32])
{
    if (!s_have_key)
    {
        return 0;
    }
    memcpy(out, s_session_key, 32);
    return 1;
}
