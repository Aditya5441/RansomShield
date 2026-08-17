/**
 * @file    ina219.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "ina219.h"
#include "bm.h"

#define INA219_ADDR (0x40 << 1)

/**
 * @brief   TODO: describe what INA219_Init() does
 */
void INA219_Init(void)
{
    uint8_t config[3] = {0x00, 0x39, 0x9F};
    bm_i2c_master_transmit(I2C1, INA219_ADDR, config, 3, 100);
}
/**
 * @brief   TODO: describe what INA219_GetVoltage() does
 * @retval  TODO: describe return value
 */
float INA219_GetVoltage(void)
{
    uint8_t reg = 0x02;
    uint8_t data[2];

    bm_i2c_master_transmit(I2C1, INA219_ADDR, &reg, 1, 100);
    bm_i2c_master_receive(I2C1, INA219_ADDR, data, 2, 100);

    int16_t raw = (int16_t)((data[0] << 8) | data[1]);
    return (float)raw * 0.001f;
}

/**
 * @brief   TODO: describe what INA219_GetCurrent() does
 * @retval  TODO: describe return value
 */
float INA219_GetCurrent(void)
{
    uint8_t reg = 0x04;
    uint8_t data[2];

    bm_i2c_master_transmit(I2C1, INA219_ADDR, &reg, 1, 100);
    bm_i2c_master_receive(I2C1, INA219_ADDR, data, 2, 100);

    int16_t raw = (data[0] << 8) | data[1];
    return raw * 0.01;
}

/**
 * @brief   TODO: describe what INA219_GetPower() does
 * @retval  TODO: describe return value
 */
float INA219_GetPower(void)
{
    return INA219_GetVoltage() * INA219_GetCurrent();
}

