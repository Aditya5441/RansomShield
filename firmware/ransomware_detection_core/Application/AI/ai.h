/**
 * @file    ai.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef RANSOMWARE_DETECTOR_H
#define RANSOMWARE_DETECTOR_H

#define RANSOMWARE_INPUT_COUNT 8
#define RANSOMWARE_THRESHOLD 0.45f

#ifdef __cplusplus
extern "C" {
#endif

int Ransomware_Init(void);
float Ransomware_Predict(const float *inputs);
int Ransomware_IsAlert(float probability);

#ifdef __cplusplus
}
#endif

#endif
