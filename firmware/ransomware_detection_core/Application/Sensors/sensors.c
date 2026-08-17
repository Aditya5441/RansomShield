/**
 * @file    sensors.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "sensors.h"

#include "ina219.h"
#include "mpu6050.h"
#include "lis3dh.h"
#include "ntc.h"
#include "voltage_sensor.h"
#include "nrf24.h"
#include "si4432.h"
#include "w5500.h"

/**
 * @brief   TODO: describe what Sensors_Init() does
 */
void Sensors_Init(void)
{
    INA219_Init();
    MPU6050_Init();
    LIS3DH_Init();
    VoltageSensor_Init();
    NRF24_Init();
    SI4432_Init();
    W5500_Init();
}

/**
 * @brief   TODO: describe what Sensors_ReadAll() does
 * @param   data  TODO: describe parameter
 */
void Sensors_ReadAll(SensorData_t *data)
{
    data->usb_voltage = VoltageSensor_Read();
    data->usb_current = INA219_GetCurrent();
    data->usb_power   = INA219_GetPower();

    data->temp = NTC_ReadTemperature();

    MPU6050_ReadAccel(
        &data->accel_external[0],
        &data->accel_external[1],
        &data->accel_external[2]
    );

    LIS3DH_ReadAccel(
        &data->accel_internal[0],
        &data->accel_internal[1],
        &data->accel_internal[2]
    );

    data->rf_nrf_dbm = NRF24_GetRSSI();
    data->rf_si_dbm  = SI4432_GetRSSI();
    data->net_traffic = W5500_GetTraffic();
}
