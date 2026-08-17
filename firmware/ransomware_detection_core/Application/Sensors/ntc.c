/**
 * @file    ntc.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "ntc.h"
#include "bm.h"
#include <math.h>

/**
 * @brief   TODO: describe what NTC_ReadTemperature() does
 * @retval  TODO: describe return value
 */
float NTC_ReadTemperature(void)
{
    uint16_t adc = 0;

    bm_adc_read_channel(4U, &adc, 100);

    float V = (adc * 3.3f) / 4095.0f;
    float R = (10000.0f * (3.3f - V)) / V;

    float temp = 1.0f / ( (1.0f/298.15f) + (1.0f/3950.0f)*log(R/10000.0f) );
    return temp - 273.15f;
}
