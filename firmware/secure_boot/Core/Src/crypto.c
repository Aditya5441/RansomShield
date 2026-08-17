#include "crypto.h"
#include <string.h>
#include "stm32wbxx.h"

/* AES-256 ECB, driven directly through the AES1 peripheral registers
 * (no stm32wbxx_hal_cryp.h). Mode = 11b (key derivation + decrypt in a
 * single pass) so the peripheral derives the decryption key from the
 * cipher key we load, without a separate derivation-only step. */
void CRYPTO_AES_Decrypt(uint8_t *in, uint8_t *key, uint8_t *out)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_AES1EN;
    (void)RCC->AHB2ENR;

    AES1->CR = 0U;   /* disable before reconfiguring */

    /* Load the 256-bit key, MSW first (KEYR7..KEYR0 = key[0..31] big-endian,
     * matching the byte order HAL_CRYP_SetKey() expects for a 32-byte key). */
    AES1->KEYR7 = ((uint32_t)key[0]  << 24) | ((uint32_t)key[1]  << 16) | ((uint32_t)key[2]  << 8) | key[3];
    AES1->KEYR6 = ((uint32_t)key[4]  << 24) | ((uint32_t)key[5]  << 16) | ((uint32_t)key[6]  << 8) | key[7];
    AES1->KEYR5 = ((uint32_t)key[8]  << 24) | ((uint32_t)key[9]  << 16) | ((uint32_t)key[10] << 8) | key[11];
    AES1->KEYR4 = ((uint32_t)key[12] << 24) | ((uint32_t)key[13] << 16) | ((uint32_t)key[14] << 8) | key[15];
    AES1->KEYR3 = ((uint32_t)key[16] << 24) | ((uint32_t)key[17] << 16) | ((uint32_t)key[18] << 8) | key[19];
    AES1->KEYR2 = ((uint32_t)key[20] << 24) | ((uint32_t)key[21] << 16) | ((uint32_t)key[22] << 8) | key[23];
    AES1->KEYR1 = ((uint32_t)key[24] << 24) | ((uint32_t)key[25] << 16) | ((uint32_t)key[26] << 8) | key[27];
    AES1->KEYR0 = ((uint32_t)key[28] << 24) | ((uint32_t)key[29] << 16) | ((uint32_t)key[30] << 8) | key[31];

    /* DATATYPE=01 (byte swap, so plain big-endian byte buffers can be pushed
     * straight into DINR/DOUTR as 32-bit words); CHMOD=000 (ECB);
     * MODE=11 (key derivation then decrypt); KEYSIZE selection for AES-256
     * is implicit on parts with a single 256-bit key bank. */
    AES1->CR = AES_CR_DATATYPE_0 | AES_CR_MODE_0 | AES_CR_MODE_1;
    AES1->CR |= AES_CR_EN;

    /* Wait for the key-derivation phase (part of MODE=11) to complete before
     * pushing the ciphertext block. */
    while ((AES1->SR & AES_SR_CCF) == 0U)
    {
    }
    AES1->CR |= AES_CR_CCFC;   /* clear CCF, ready for the data phase */

    /* Push one 16-byte ciphertext block (4 words). */
    for (int w = 0; w < 4; w++)
    {
        const uint8_t *p = in + (w * 4);
        uint32_t word = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
        AES1->DINR = word;
    }

    while ((AES1->SR & AES_SR_CCF) == 0U)
    {
    }

    for (int w = 0; w < 4; w++)
    {
        uint32_t word = AES1->DOUTR;
        out[w * 4 + 0] = (uint8_t)(word >> 24);
        out[w * 4 + 1] = (uint8_t)(word >> 16);
        out[w * 4 + 2] = (uint8_t)(word >> 8);
        out[w * 4 + 3] = (uint8_t)(word);
    }

    AES1->CR |= AES_CR_CCFC;
    AES1->CR &= ~AES_CR_EN;
}

/* SHA-256 software implementation (FIPS 180-4). Self-contained, no
 * dependency on a hardware HASH peripheral or on HAL - unchanged from the
 * original, it never used HAL to begin with. */

static const uint32_t K[64] = {
    0x428a2f98UL,0x71374491UL,0xb5c0fbcfUL,0xe9b5dba5UL,
    0x3956c25bUL,0x59f111f1UL,0x923f82a4UL,0xab1c5ed5UL,
    0xd807aa98UL,0x12835b01UL,0x243185beUL,0x550c7dc3UL,
    0x72be5d74UL,0x80deb1feUL,0x9bdc06a7UL,0xc19bf174UL,
    0xe49b69c1UL,0xefbe4786UL,0x0fc19dc6UL,0x240ca1ccUL,
    0x2de92c6fUL,0x4a7484aaUL,0x5cb0a9dcUL,0x76f988daUL,
    0x983e5152UL,0xa831c66dUL,0xb00327c8UL,0xbf597fc7UL,
    0xc6e00bf3UL,0xd5a79147UL,0x06ca6351UL,0x14292967UL,
    0x27b70a85UL,0x2e1b2138UL,0x4d2c6dfcUL,0x53380d13UL,
    0x650a7354UL,0x766a0abbUL,0x81c2c92eUL,0x92722c85UL,
    0xa2bfe8a1UL,0xa81a664bUL,0xc24b8b70UL,0xc76c51a3UL,
    0xd192e819UL,0xd6990624UL,0xf40e3585UL,0x106aa070UL,
    0x19a4c116UL,0x1e376c08UL,0x2748774cUL,0x34b0bcb5UL,
    0x391c0cb3UL,0x4ed8aa4aUL,0x5b9cca4fUL,0x682e6ff3UL,
    0x748f82eeUL,0x78a5636fUL,0x84c87814UL,0x8cc70208UL,
    0x90befffaUL,0xa4506cebUL,0xbef9a3f7UL,0xc67178f2UL
};

#define ROTR(x, n)  (((x) >> (n)) | ((x) << (32u - (n))))
#define CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define BSIG0(x) (ROTR(x,2)  ^ ROTR(x,13) ^ ROTR(x,22))
#define BSIG1(x) (ROTR(x,6)  ^ ROTR(x,11) ^ ROTR(x,25))
#define SSIG0(x) (ROTR(x,7)  ^ ROTR(x,18) ^ ((x) >> 3))
#define SSIG1(x) (ROTR(x,17) ^ ROTR(x,19) ^ ((x) >> 10))

static void sha256_process_block(uint32_t h[8], const uint8_t block[64])
{
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, hh;
    int i;

    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i * 4]     << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8)  |
               ((uint32_t)block[i * 4 + 3]);
    }
    for (i = 16; i < 64; i++) {
        w[i] = SSIG1(w[i - 2]) + w[i - 7] + SSIG0(w[i - 15]) + w[i - 16];
    }

    a = h[0]; b = h[1]; c = h[2]; d = h[3];
    e = h[4]; f = h[5]; g = h[6]; hh = h[7];

    for (i = 0; i < 64; i++) {
        uint32_t t1 = hh + BSIG1(e) + CH(e, f, g) + K[i] + w[i];
        uint32_t t2 = BSIG0(a) + MAJ(a, b, c);
        hh = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
    h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
}

void CRYPTO_SHA256(const uint8_t *data, uint32_t len, uint8_t *hash)
{
    uint32_t h[8] = {
        0x6a09e667UL, 0xbb67ae85UL, 0x3c6ef372UL, 0xa54ff53aUL,
        0x510e527fUL, 0x9b05688cUL, 0x1f83d9abUL, 0x5be0cd19UL
    };
    uint64_t bit_len = (uint64_t)len * 8U;
    uint32_t full_blocks = len / 64U;
    uint32_t rem = len % 64U;
    uint32_t i;
    uint8_t pad[128];
    uint32_t pad_len;

    for (i = 0; i < full_blocks; i++) {
        sha256_process_block(h, data + (i * 64U));
    }

    memset(pad, 0, sizeof(pad));
    memcpy(pad, data + (full_blocks * 64U), rem);
    pad[rem] = 0x80U;

    pad_len = (rem < 56U) ? 64U : 128U;

    pad[pad_len - 8] = (uint8_t)(bit_len >> 56);
    pad[pad_len - 7] = (uint8_t)(bit_len >> 48);
    pad[pad_len - 6] = (uint8_t)(bit_len >> 40);
    pad[pad_len - 5] = (uint8_t)(bit_len >> 32);
    pad[pad_len - 4] = (uint8_t)(bit_len >> 24);
    pad[pad_len - 3] = (uint8_t)(bit_len >> 16);
    pad[pad_len - 2] = (uint8_t)(bit_len >> 8);
    pad[pad_len - 1] = (uint8_t)(bit_len);

    sha256_process_block(h, pad);
    if (pad_len == 128U) {
        sha256_process_block(h, pad + 64U);
    }

    for (i = 0; i < 8U; i++) {
        hash[i * 4]     = (uint8_t)(h[i] >> 24);
        hash[i * 4 + 1] = (uint8_t)(h[i] >> 16);
        hash[i * 4 + 2] = (uint8_t)(h[i] >> 8);
        hash[i * 4 + 3] = (uint8_t)(h[i]);
    }
}
