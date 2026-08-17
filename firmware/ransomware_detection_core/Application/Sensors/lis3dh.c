/**
 * @file    lis3dh.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "lis3dh.h"
#include "bm.h"

#define LIS_ADDR (0x18 << 1)

/**
 * @brief   TODO: describe what LIS3DH_Init() does
 */
void LIS3DH_Init(void)
{
    uint8_t data[2] = {0x20, 0x57};
    bm_i2c_master_transmit(I2C1, LIS_ADDR, data, 2, 100);
}

/**
 * @brief   TODO: describe what LIS3DH_ReadAccel() does
 * @param   x  TODO: describe parameter
 * @param   y  TODO: describe parameter
 * @param   z  TODO: describe parameter
 */
void LIS3DH_ReadAccel(float *x, float *y, float *z)
{
    uint8_t reg = 0x28 | 0x80;
    uint8_t data[6];

    bm_i2c_master_transmit(I2C1, LIS_ADDR, &reg, 1, 100);
    bm_i2c_master_receive(I2C1, LIS_ADDR, data, 6, 100);

    int16_t rx = (data[1]<<8)|data[0];
    int16_t ry = (data[3]<<8)|data[2];
    int16_t rz = (data[5]<<8)|data[4];

    *x = rx / 16384.0f;
    *y = ry / 16384.0f;
    *z = rz / 16384.0f;
}

