/**
 * @file    cs.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_DSP_CS_H_
#define APPLICATION_DSP_CS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CS_SIZE 64
#define CS_MEASUREMENTS 32

void CS_Init(void);
void CS_Compress(float *input, float *output);
void CS_Reconstruct(float *compressed, float *reconstructed);

#ifdef __cplusplus
}
#endif

#endif
