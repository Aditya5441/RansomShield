/**
 * @file    dsp.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_DSP_DSP_H_
#define APPLICATION_DSP_DSP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DSP_SIZE 64

typedef struct
{
    float mean;
    float variance;
    float energy;
    float entropy;
    float peak;
    float chaos;
    float spectral_energy;
    float spectral_centroid;
    float wvd_energy;
    float wvd_ridge;
} DSP_Features_t;

void DSP_Init(void);
void DSP_RunPipeline(float *input, DSP_Features_t *features);

#ifdef __cplusplus
}
#endif

#endif
