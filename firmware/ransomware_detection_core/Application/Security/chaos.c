/**
 * @file    chaos.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "chaos.h"
#include "bm.h"
#include <math.h>

/**
 * @brief   TODO: describe what logistic() does
 * @param   x  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static float logistic(float x)
{
    return 3.99f * x * (1.0f - x);
}

/**
 * @brief   TODO: describe what tent() does
 * @param   x  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static float tent(float x)
{
    if (x < 0.5f) {
        return 2.0f * x;
    }
    return 2.0f * (1.0f - x);
}

/**
 * @brief   TODO: describe what sine_map() does
 * @param   x  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static float sine_map(float x)
{
    return sinf(3.1415926f * x);
}

/**
 * @brief   TODO: describe what Chaos_Init() does
 * @param   state  TODO: describe parameter
 */
void Chaos_Init(ChaosState_t *state)
{
    uint32_t r1, r2, r3;

    bm_rng_read_u32(&r1, 100);
    bm_rng_read_u32(&r2, 100);
    bm_rng_read_u32(&r3, 100);

    state->x = (float)(r1 % 1000) / 1000.0f;
    state->y = (float)(r2 % 1000) / 1000.0f;
    state->z = (float)(r3 % 1000) / 1000.0f;

    if (state->x == 0) {
        state->x = 0.123f;
    }
    if (state->y == 0) {
        state->y = 0.456f;
    }
    if (state->z == 0) {
        state->z = 0.789f;
    }
}

/**
 * @brief   TODO: describe what Chaos_Next() does
 * @param   s  TODO: describe parameter
 * @retval  TODO: describe return value
 */
float Chaos_Next(ChaosState_t *s)
{
    s->x = logistic(s->x);
    s->y = tent(s->y);
    s->z = sine_map(s->z);

    float new_x = fmodf(s->x + 0.3f * s->y, 1.0f);
    float new_y = fmodf(s->y + 0.3f * s->z, 1.0f);
    float new_z = fmodf(s->z + 0.3f * s->x, 1.0f);

    s->x = new_x;
    s->y = new_y;
    s->z = new_z;

    return fmodf(s->x + s->y + s->z, 1.0f);
}

/**
 * @brief   TODO: describe what Chaos_GenerateVector() does
 * @param   state  TODO: describe parameter
 * @param   out  TODO: describe parameter
 * @param   len  TODO: describe parameter
 */
void Chaos_GenerateVector(ChaosState_t *state, float *out, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        out[i] = Chaos_Next(state);
    }
}

/**
 * @brief   TODO: describe what Chaos_MixInt() does
 * @param   data  TODO: describe parameter
 * @param   len  TODO: describe parameter
 */
void Chaos_MixInt(int32_t *data, uint16_t len)
{
    ChaosState_t state;
    Chaos_Init(&state);

    for (uint16_t i = 0; i < len; i++) {
        float c = Chaos_Next(&state);
        uint32_t mask = (uint32_t)(c * 0xFFFFFFFF);

        data[i] ^= mask;

        uint8_t shift = mask % 16;
        data[i] = (data[i] << shift) | ((uint32_t)data[i] >> (32 - shift));
        data[i] *= 2654435761U;
    }
}

/**
 * @brief   TODO: describe what Chaos_MixFloat() does
 * @param   data  TODO: describe parameter
 * @param   len  TODO: describe parameter
 */
void Chaos_MixFloat(float *data, uint16_t len)
{
    ChaosState_t state;
    Chaos_Init(&state);

    for (uint16_t i = 0; i < len; i++) {
        float c = Chaos_Next(&state);

        data[i] = data[i] * (1.0f + c);
        data[i] += sinf(c * 10.0f);

        if (i > 0) {
            data[i] += 0.2f * data[i - 1];
        }
    }
}
