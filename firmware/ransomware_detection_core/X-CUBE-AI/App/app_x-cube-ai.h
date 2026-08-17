/**
 * @file    app_x-cube-ai.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APP_X_CUBE_AI_H
#define APP_X_CUBE_AI_H

#include "sensors.h"

#ifdef __cplusplus
extern "C" {
#endif

void ai_init(void);
float ai_predict(const float *inputs, int count);
float ai_predict_sensors(const SensorData_t *sd);
int ai_is_alert(float probability);

#ifdef __cplusplus
}
#endif

#endif
