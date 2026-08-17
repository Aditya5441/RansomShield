/**
 * @file    ai.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "ai.h"
#include "ransomware_weights.h"
#include <math.h>

#define HIDDEN1 64
#define HIDDEN2 32

static float hidden1[HIDDEN1];
static float hidden2[HIDDEN2];

/**
 * @brief   TODO: describe what relu() does
 * @param   x  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static float relu(float x)
{
    return (x > 0.0f) ? x : 0.0f;
}

/**
 * @brief   TODO: describe what sigmoid() does
 * @param   x  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static float sigmoid(float x)
{
    if (x < -40.0f) {
        return 0.0f;
    }
    if (x > 40.0f) {
        return 1.0f;
    }
    return 1.0f / (1.0f + expf(-x));
}

/**
 * @brief   TODO: describe what Ransomware_Init() does
 * @retval  TODO: describe return value
 */
int Ransomware_Init(void)
{
    return 0;
}

/**
 * @brief   TODO: describe what Ransomware_Predict() does
 * @param   inputs  TODO: describe parameter
 * @retval  TODO: describe return value
 */
float Ransomware_Predict(const float *inputs)
{
    float normalized[RANSOMWARE_INPUT_COUNT];
    int i;
    int j;

    for (i = 0; i < RANSOMWARE_INPUT_COUNT; i++) {
        normalized[i] = (inputs[i] - ransomware_norm_mean[i]) * ransomware_norm_inv_std[i];
    }

    for (j = 0; j < HIDDEN1; j++) {
        float sum = ransomware_b1[j];
        for (i = 0; i < RANSOMWARE_INPUT_COUNT; i++) {
            sum += ransomware_w1[j * RANSOMWARE_INPUT_COUNT + i] * normalized[i];
        }
        hidden1[j] = relu(sum);
    }

    for (j = 0; j < HIDDEN2; j++) {
        float sum = ransomware_b2[j];
        for (i = 0; i < HIDDEN1; i++) {
            sum += ransomware_w2[j * HIDDEN1 + i] * hidden1[i];
        }
        hidden2[j] = relu(sum);
    }

    {
        float sum = ransomware_b3[0];
        for (i = 0; i < HIDDEN2; i++) {
            sum += ransomware_w3[i] * hidden2[i];
        }
        return sigmoid(sum);
    }
}

/**
 * @brief   TODO: describe what Ransomware_IsAlert() does
 * @param   probability  TODO: describe parameter
 * @retval  TODO: describe return value
 */
int Ransomware_IsAlert(float probability)
{
    return (probability >= RANSOMWARE_THRESHOLD) ? 1 : 0;
}
