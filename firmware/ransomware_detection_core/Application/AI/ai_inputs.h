/**
 * @file    ai_inputs.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef AI_INPUTS_H
#define AI_INPUTS_H

#include "sensors.h"

#define AI_INPUT_COUNT 8

#ifdef __cplusplus
extern "C" {
#endif

void AI_BuildSensorInput(const SensorData_t *sd, float *out);

#ifdef __cplusplus
}
#endif

#endif
