/**
 * @file    voltage.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "voltage_sensor.h"
#include "bm.h"

#define ADC_RESOLUTION   4095.0f
#define VREF             3.3f
#define R1 30000.0f
#define R2 7500.0f

/**
 * @brief   TODO: describe what VoltageSensor_Init() does
 */
void VoltageSensor_Init(void)
{
}

/**
 * @brief   TODO: describe what VoltageSensor_Read() does
 * @retval  TODO: describe return value
 */
float VoltageSensor_Read(void)
{
    uint16_t adc_val = 0;

    bm_adc_read_channel(4U, &adc_val, 100);

    float v_adc = (adc_val / ADC_RESOLUTION) * VREF;
    float v_in = v_adc * ((R1 + R2) / R2);

    return v_in;
}
