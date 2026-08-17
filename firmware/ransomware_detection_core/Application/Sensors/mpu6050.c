/**
 * @file    mpu6050.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "mpu6050.h"
#include "bm.h"

#define MPU_ADDR (0x68 << 1)

/**
 * @brief   TODO: describe what MPU6050_Init() does
 */
void MPU6050_Init(void)
{
    uint8_t data[2];

    data[0] = 0x6B;
    data[1] = 0x00;
    bm_i2c_master_transmit(I2C1, MPU_ADDR, data, 2, 100);
}

/**
 * @brief   TODO: describe what MPU6050_ReadAccel() does
 * @param   ax  TODO: describe parameter
 * @param   ay  TODO: describe parameter
 * @param   az  TODO: describe parameter
 */
void MPU6050_ReadAccel(float *ax, float *ay, float *az)
{
    uint8_t reg = 0x3B;
    uint8_t data[6];

    bm_i2c_master_transmit(I2C1, MPU_ADDR, &reg, 1, 100);
    bm_i2c_master_receive(I2C1, MPU_ADDR, data, 6, 100);

    int16_t x = (data[0]<<8)|data[1];
    int16_t y = (data[2]<<8)|data[3];
    int16_t z = (data[4]<<8)|data[5];

    *ax = x / 16384.0f;
    *ay = y / 16384.0f;
    *az = z / 16384.0f;
}
