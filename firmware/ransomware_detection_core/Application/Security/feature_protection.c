/**
 * @file    feature_protection.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "feature_protection.h"
#include "bm.h"
#include <string.h>
#include <math.h>
#include <stdint.h>

#define SCALE_FACTOR    1000.0f
#define MIX_CONST       2654435761U
#define FEATURE_COUNT   8
#define FEATURE_BUF_SIZE (FEATURE_COUNT * 4)

/**
 * @brief   TODO: describe what FS_BitLevelMix() does
 * @param   f  TODO: describe parameter
 * @param   features  TODO: describe parameter
 */
static void FS_BitLevelMix(int32_t *f, const DSP_Features_t *features)
{
    uint32_t wvd_mask =
        ((uint32_t)(features->wvd_energy * 1000.0f)) ^
        (((uint32_t)(features->wvd_ridge * 1000.0f)) << 16);

    for (int i = 0; i < FEATURE_COUNT; i++) {
        uint32_t r;
        bm_rng_read_u32(&r, 100);

        uint32_t mix = r ^ wvd_mask ^ (uint32_t)i;
        uint8_t shift = mix % 16;

        f[i] = (f[i] << shift) | ((uint32_t)f[i] >> (32 - shift));
        f[i] ^= mix;
        f[i] = f[i] * MIX_CONST;
    }
}

/**
 * @brief   TODO: describe what FS_Nonlinear() does
 * @param   f  TODO: describe parameter
 */
static void FS_Nonlinear(int32_t *f)
{
    for (int i = 0; i < FEATURE_COUNT; i++) {
        float x = (float)f[i] / SCALE_FACTOR;
        x = sinf(x) + cosf(x * 0.5f);
        f[i] = (int32_t)(x * SCALE_FACTOR);
    }
}

/**
 * @brief   TODO: describe what FS_CrossCouple() does
 * @param   f  TODO: describe parameter
 */
static void FS_CrossCouple(int32_t *f)
{
    for (int i = 1; i < FEATURE_COUNT; i++) {
        f[i] += (f[i - 1] >> 1);
    }
}

/**
 * @brief   TODO: describe what FS_MemoryScatter() does
 * @param   f  TODO: describe parameter
 * @param   buf  TODO: describe parameter
 */
static void FS_MemoryScatter(int32_t *f, uint8_t *buf)
{
    memset(buf, 0, FEATURE_BUF_SIZE);

    for (int i = 0; i < FEATURE_COUNT; i++) {
        buf[(i * 3)  % FEATURE_BUF_SIZE] ^= (uint8_t)((f[i] >> 0)  & 0xFF);
        buf[(i * 5)  % FEATURE_BUF_SIZE] ^= (uint8_t)((f[i] >> 8)  & 0xFF);
        buf[(i * 7)  % FEATURE_BUF_SIZE] ^= (uint8_t)((f[i] >> 16) & 0xFF);
        buf[(i * 11) % FEATURE_BUF_SIZE] ^= (uint8_t)((f[i] >> 24) & 0xFF);
    }
}

/**
 * @brief   TODO: describe what FS_LowLevelProtect() does
 * @param   features  TODO: describe parameter
 */
void FS_LowLevelProtect(DSP_Features_t *features)
{
    int32_t q[FEATURE_COUNT];
    float *f = (float *)features;

    for (int i = 0; i < FEATURE_COUNT; i++) {
        q[i] = (int32_t)(f[i] * SCALE_FACTOR);
    }

    FS_BitLevelMix(q, features);
    FS_Nonlinear(q);
    FS_CrossCouple(q);

    uint8_t buffer[FEATURE_BUF_SIZE];
    FS_MemoryScatter(q, buffer);
    memcpy(features, buffer, FEATURE_BUF_SIZE);
}

/**
 * @brief   TODO: describe what FS_DebugRestore() does
 * @param   features  TODO: describe parameter
 */
void FS_DebugRestore(DSP_Features_t *features)
{
    uint8_t *buf = (uint8_t *)features;
    int32_t q[FEATURE_COUNT] = {0};

    for (int i = 0; i < FEATURE_COUNT; i++) {
        q[i] |= ((int32_t)buf[(i * 3)  % FEATURE_BUF_SIZE]);
        q[i] |= ((int32_t)buf[(i * 5)  % FEATURE_BUF_SIZE]) << 8;
        q[i] |= ((int32_t)buf[(i * 7)  % FEATURE_BUF_SIZE]) << 16;
        q[i] |= ((int32_t)buf[(i * 11) % FEATURE_BUF_SIZE]) << 24;
    }

    for (int i = 0; i < FEATURE_COUNT; i++) {
        ((float *)features)[i] = (float)q[i] / SCALE_FACTOR;
    }
}
