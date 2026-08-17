/**
 * @file    ecdh.c
 * @brief   See ecdh.h. Point multiplication (keypair gen and shared-
 *          secret computation) both go through HAL_PKA_ECCMul - ECDH
 *          is just "scalar * point", so the same PKA call does both:
 *          scalar * G for keygen, scalar * peer_pub for the shared
 *          secret.
 */
#include "ecdh.h"
#include "bm.h"
#include "stm32wbxx_hal.h"
#include <string.h>

extern PKA_HandleTypeDef hpka;

/* NIST P-256 (secp256r1) domain parameters, big-endian, from FIPS
 * 186-4 / SEC2 - these are the standard published curve constants,
 * not project-specific secrets. */
static const uint8_t P256_PRIME[32] = {
    0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};
static const uint8_t P256_A[32] = {
    0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC
};
static const uint8_t P256_GX[32] = {
    0x6B,0x17,0xD1,0xF2,0xE1,0x2C,0x42,0x47,0xF8,0xBC,0xE6,0xE5,0x63,0xA4,0x40,0xF2,
    0x77,0x03,0x7D,0x81,0x2D,0xEB,0x33,0xA0,0xF4,0xA1,0x39,0x45,0xD8,0x98,0xC2,0x96
};
static const uint8_t P256_GY[32] = {
    0x4F,0xE3,0x42,0xE2,0xFE,0x1A,0x7F,0x9B,0x8E,0xE7,0xEB,0x4A,0x7C,0x0F,0x9E,0x16,
    0x2B,0xCE,0x33,0x57,0x6B,0x31,0x5E,0xCE,0xCB,0xB6,0x40,0x68,0x37,0xBF,0x51,0xF5
};
static const uint8_t P256_ORDER[32] = {
    0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xBC,0xE6,0xFA,0xAD,0xA7,0x17,0x9E,0x84,0xF3,0xB9,0xCA,0xC2,0xFC,0x63,0x25,0x51
};

/**
 * @brief   scalar * (in_x, in_y) -> (out_x, out_y) via the PKA's ECC
 *          point-multiply mode. This one function does double duty:
 *          call with (priv, Gx, Gy) for keygen, or (priv, peer_x,
 *          peer_y) for the shared secret.
 */
static void ecc_point_mul(const uint8_t scalar[32],
                           const uint8_t in_x[32], const uint8_t in_y[32],
                           uint8_t out_x[32], uint8_t out_y[32])
{
    PKA_ECCMulInTypeDef mul_in = {0};

    mul_in.primeOrderSize = 32U;
    mul_in.modulusSize    = 32U;
    mul_in.coefSign       = 0;
    mul_in.coefA          = P256_A;
    mul_in.modulus        = P256_PRIME;
    mul_in.pointX         = in_x;
    mul_in.pointY         = in_y;
    mul_in.scalarMul      = scalar;

    HAL_PKA_ECCMul(&hpka, &mul_in, HAL_MAX_DELAY);
    HAL_PKA_ECCMul_GetResult(&hpka, out_x, out_y);
}

void ECDH_GenerateKeypair(uint8_t priv[ECDH_PRIV_LEN], uint8_t pub[ECDH_PUB_LEN])
{
    /* Reject a zero/degenerate scalar and anything >= curve order -
     * astronomically unlikely from a good RNG, but cheap to check. */
    do {
        for (int i = 0; i < 8; i++) {
            uint32_t r;
            bm_rng_read_u32(&r, 100);
            priv[i * 4]     = (uint8_t)(r >> 24);
            priv[i * 4 + 1] = (uint8_t)(r >> 16);
            priv[i * 4 + 2] = (uint8_t)(r >> 8);
            priv[i * 4 + 3] = (uint8_t)(r);
        }
    } while ((memcmp(priv, (const uint8_t[32]){0}, 32) == 0) ||
             (memcmp(priv, P256_ORDER, 32) >= 0));

    ecc_point_mul(priv, P256_GX, P256_GY, pub, pub + 32);
}

void ECDH_ComputeShared(const uint8_t priv[ECDH_PRIV_LEN],
                         const uint8_t peer_pub[ECDH_PUB_LEN],
                         uint8_t shared_x[32])
{
    uint8_t shared_y[32];
    /* Note: no public-key validation here (on-curve check, not the
     * point at infinity, not a known low-order point). For a real
     * deployment add that check before trusting peer_pub - accepting
     * an unvalidated peer point opens the door to invalid-curve
     * attacks. Flagging rather than silently shipping it. */
    ecc_point_mul(priv, peer_pub, peer_pub + 32, shared_x, shared_y);
}
