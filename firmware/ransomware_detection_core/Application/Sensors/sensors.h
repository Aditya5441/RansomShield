/**
 * @file    sensors.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_SENSORS_SENSORS_H_
#define APPLICATION_SENSORS_SENSORS_H_

#include <stdint.h>

typedef struct {
    float usb_voltage;
    float usb_current;
    float usb_power;
    float temp;
    float accel_external[3];
    float accel_internal[3];
    int8_t rf_nrf_dbm;
    int8_t rf_si_dbm;
    float net_traffic;
} SensorData_t;

void Sensors_Init(void);
void Sensors_ReadAll(SensorData_t *data);

#endif
