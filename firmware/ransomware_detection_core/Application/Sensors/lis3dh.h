/**
 * @file    lis3dh.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_SENSORS_LIS3DH_H_
#define APPLICATION_SENSORS_LIS3DH_H_

void LIS3DH_Init(void);
void LIS3DH_ReadAccel(float *x, float *y, float *z);

#endif
