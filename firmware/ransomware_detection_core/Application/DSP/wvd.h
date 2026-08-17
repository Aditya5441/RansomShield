/**
 * @file    wvd.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_DSP_WVD_H_
#define APPLICATION_DSP_WVD_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WVD_SIZE 64

void WVD_Compute(float *signal, float wvd[WVD_SIZE][WVD_SIZE]);

#ifdef __cplusplus
}
#endif

#endif
