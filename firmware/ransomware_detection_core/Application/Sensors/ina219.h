/**
 * @file    ina219.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_SENSORS_INA219_H_
#define APPLICATION_SENSORS_INA219_H_

void INA219_Init(void);
float INA219_GetVoltage(void);
float INA219_GetCurrent(void);
float INA219_GetPower(void);

#endif
