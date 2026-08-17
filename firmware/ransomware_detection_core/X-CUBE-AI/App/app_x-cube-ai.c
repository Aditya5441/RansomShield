/**
 * @file    app_x-cube-ai.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "app_x-cube-ai.h"
#include "ai.h"
#include "ai_inputs.h"

/**
 * @brief   TODO: describe what ai_init() does
 */
void ai_init(void)
{
    Ransomware_Init();
}

/**
 * @brief   TODO: describe what ai_predict() does
 * @param   inputs  TODO: describe parameter
 * @param   count  TODO: describe parameter
 * @retval  TODO: describe return value
 */
float ai_predict(const float *inputs, int count)
{
    float sensor_inputs[RANSOMWARE_INPUT_COUNT];

    if (count >= RANSOMWARE_INPUT_COUNT) {
        return Ransomware_Predict(inputs);
    }

    for (int i = 0; i < RANSOMWARE_INPUT_COUNT; i++) {
        sensor_inputs[i] = (i < count) ? inputs[i] : 0.0f;
    }

    return Ransomware_Predict(sensor_inputs);
}

/**
 * @brief   TODO: describe what ai_predict_sensors() does
 * @param   sd  TODO: describe parameter
 * @retval  TODO: describe return value
 */
float ai_predict_sensors(const SensorData_t *sd)
{
    float inputs[RANSOMWARE_INPUT_COUNT];

    AI_BuildSensorInput(sd, inputs);
    return Ransomware_Predict(inputs);
}

/**
 * @brief   TODO: describe what ai_is_alert() does
 * @param   probability  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int ai_is_alert(float probability)
{
    return Ransomware_IsAlert(probability);
}
