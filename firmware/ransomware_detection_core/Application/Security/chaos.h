/**
 * @file    chaos.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_SECURITY_CHAOS_H_
#define APPLICATION_SECURITY_CHAOS_H_
#include <stdint.h>

typedef struct
{
    float x;
    float y;
    float z;
} ChaosState_t;

void Chaos_Init(ChaosState_t *state);

float Chaos_Next(ChaosState_t *state);

void Chaos_GenerateVector(ChaosState_t *state, float *out, uint16_t len);

void Chaos_MixInt(int32_t *data, uint16_t len);

void Chaos_MixFloat(float *data, uint16_t len);

#endif
