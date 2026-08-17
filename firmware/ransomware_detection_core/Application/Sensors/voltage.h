/**
 * @file    voltage.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_SENSORS_VOLTAGE_H_
#define APPLICATION_SENSORS_VOLTAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

void VoltageSensor_Init(void);
float VoltageSensor_Read(void);

#ifdef __cplusplus
}
#endif

#endif
