/**
 * @file    ecdsa_verify.c
 * @brief   See ecdsa_verify.h. BAREMETAL port: talks to the PKA peripheral
 *          directly (PKA->CR/SR/RAM) instead of stm32wbxx_hal_pka.h.
 *
 * PKA RAM operand layout for the ECDSA-verification primitive (mode 0x26)
 * below matches ST's own PKA v1 IP block (shared unmodified across
 * STM32WB/WL/L5/U5): offsets confirmed against ST's stm32wlxx_crypto_pkc.c /
 * stm32l5xx_crypto_pkc.c reference porting layers and the stm32wlxx-hal
 * (Rust) driver's ECDSA verify implementation, which target the same PKA
 * core used on the WB55. Each operand is stored in PKA RAM as 8 words in
 * ascending-significance order (least-significant word first), which is
 * the opposite order from how the byte arrays below are written (MSB
 * first) - pka_write_be() below does that conversion.
 */
#include "ecdsa_verify.h"
#include "stm32wbxx.h"
#include <string.h>

/* Same NIST P-256 domain parameters as before. */
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

/* PKA operation code for ECDSA verification (RM0434 PKA_CR.MODE). */
#define PKA_MODE_ECDSA_VERIFY   0x26U

/* PKA RAM word indices (PKA->RAM[i], each i = one 32-bit word,
 * PKA->RAM base is already offset 0x400 from the peripheral base in the
 * CMSIS PKA_TypeDef, so these are byte-offset-from-0x400 divided by 4). */
#define PKA_RAM_IDX(byte_off_from_0x400)  (((byte_off_from_0x400)) / 4U)

#define IDX_N_LEN     PKA_RAM_IDX(0x004)   /* order n bit length   */
#define IDX_P_LEN     PKA_RAM_IDX(0x0B4)   /* modulus p bit length */
#define IDX_A_SIGN    PKA_RAM_IDX(0x05C)   /* curve coeff a sign   */
#define IDX_A         PKA_RAM_IDX(0x060)   /* curve coeff |a|      */
#define IDX_P         PKA_RAM_IDX(0x0B8)   /* modulus p            */
#define IDX_GX        PKA_RAM_IDX(0x1E8)   /* base point Gx        */
#define IDX_GY        PKA_RAM_IDX(0x23C)   /* base point Gy        */
#define IDX_QX        PKA_RAM_IDX(0xB40)   /* public key Xq        */
#define IDX_QY        PKA_RAM_IDX(0xB94)   /* public key Yq        */
#define IDX_SIG_R     PKA_RAM_IDX(0xC98)   /* signature R          */
#define IDX_SIG_S     PKA_RAM_IDX(0x644)   /* signature S          */
#define IDX_HASH      PKA_RAM_IDX(0xBE8)   /* message hash         */
#define IDX_ORDER     PKA_RAM_IDX(0x95C)   /* curve order n        */
#define IDX_RESULT    PKA_RAM_IDX(0x1B0)   /* 0 = valid signature  */

#define P256_WORDS    8U   /* 256 bits / 32 = 8 words per operand */

/**
 * @brief  Write a 32-byte big-endian value (MSB-first byte array, as used
 *         throughout this codebase for keys/hashes/signatures) into PKA
 *         RAM starting at word index `idx`, converting it to the
 *         least-significant-word-first order the PKA core expects.
 */
static void pka_write_be32(uint32_t idx, const uint8_t be[32])
{
    for (uint32_t w = 0; w < P256_WORDS; w++)
    {
        /* word w=0 (LSW) comes from the LAST 4 bytes of the big-endian
         * array; word w=7 (MSW) comes from the FIRST 4 bytes. */
        const uint8_t *p = be + (P256_WORDS - 1U - w) * 4U;
        uint32_t val = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
                       ((uint32_t)p[2] << 8)  | (uint32_t)p[3];
        PKA->RAM[idx + w] = val;
    }
}

static void pka_zero_ram(void)
{
    for (uint32_t i = 0; i < 894U; i++)
    {
        PKA->RAM[i] = 0U;
    }
}

int ECDSA_VerifyP256(const uint8_t pubkey[ECDSA_PUBKEY_LEN],
                      const uint8_t hash[ECDSA_HASH_LEN],
                      const uint8_t signature[ECDSA_SIGNATURE_LEN])
{
    RCC->AHB3ENR |= RCC_AHB3ENR_PKAEN;
    (void)RCC->AHB3ENR;

    PKA->CR = 0U;
    /* Setting EN triggers an automatic 894-cycle hardware clear of PKA RAM;
     * writes to EN are ignored while that's in progress, so re-assert it
     * until it sticks (same pattern ST's own PKA drivers use). */
    PKA->CR = PKA_CR_EN;
    while ((PKA->CR & PKA_CR_EN) == 0U)
    {
        PKA->CR = PKA_CR_EN;
    }
    /* Belt-and-braces: zero RAM ourselves too so this function doesn't
     * depend on that hardware-clear timing detail. */
    pka_zero_ram();

    PKA->RAM[IDX_N_LEN] = 256U;   /* prime order n is 256 bits */
    PKA->RAM[IDX_P_LEN] = 256U;   /* modulus p is 256 bits     */
    /* P256_A[] already holds the positive residue (p - 3 mod p), matching
     * how the original HAL code passed it with coefSign = 0 (positive). */
    PKA->RAM[IDX_A_SIGN] = 0U;

    pka_write_be32(IDX_A,     P256_A);
    pka_write_be32(IDX_P,     P256_PRIME);
    pka_write_be32(IDX_GX,    P256_GX);
    pka_write_be32(IDX_GY,    P256_GY);
    pka_write_be32(IDX_QX,    pubkey);
    pka_write_be32(IDX_QY,    pubkey + 32);
    pka_write_be32(IDX_SIG_R, signature);
    pka_write_be32(IDX_SIG_S, signature + 32);
    pka_write_be32(IDX_HASH,  hash);
    pka_write_be32(IDX_ORDER, P256_ORDER);

    if ((PKA->SR & (PKA_SR_ADDRERRF | PKA_SR_RAMERRF)) != 0U)
    {
        PKA->CLRFR = PKA_SR_ADDRERRF | PKA_SR_RAMERRF;
        PKA->CR &= ~PKA_CR_EN;
        return 0;   /* fail closed */
    }

    /* Start the ECDSA verify operation. */
    PKA->CR = PKA_CR_EN | (PKA_MODE_ECDSA_VERIFY << PKA_CR_MODE_Pos) | PKA_CR_START;

    /* Poll for completion (no timeout guard here mirrors the original
     * HAL_MAX_DELAY blocking call - a stuck PKA on a secure-boot stage is
     * already an unrecoverable hardware fault). */
    while ((PKA->SR & PKA_SR_PROCENDF) == 0U)
    {
        if ((PKA->SR & (PKA_SR_ADDRERRF | PKA_SR_RAMERRF)) != 0U)
        {
            PKA->CLRFR = PKA_SR_ADDRERRF | PKA_SR_RAMERRF;
            PKA->CR &= ~PKA_CR_EN;
            return 0;   /* PKA error - fail closed */
        }
    }

    uint32_t result = PKA->RAM[IDX_RESULT];

    PKA->CLRFR = PKA_SR_PROCENDF;
    PKA->CR &= ~PKA_CR_EN;

    return (result == 0U) ? 1 : 0;   /* 0 = valid signature (RM0434 Table "ECDSA verification - Outputs") */
}
