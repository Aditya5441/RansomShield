/**
 * @file    mpu6050.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_SENSORS_MPU6050_H_
#define APPLICATION_SENSORS_MPU6050_H_

void MPU6050_Init(void);
void MPU6050_ReadAccel(float *ax, float *ay, float *az);

#endif
