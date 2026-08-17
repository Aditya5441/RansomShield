/**
 * @file    ai_inputs.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "ai_inputs.h"
#include <math.h>

/**
 * @brief   TODO: describe what AI_BuildSensorInput() does
 * @param   sd  TODO: describe parameter
 * @param   out  TODO: describe parameter
 */
void AI_BuildSensorInput(const SensorData_t *sd, float *out)
{
    out[0] = sd->usb_voltage;
    out[1] = sd->usb_current;
    out[2] = sd->usb_power;
    out[3] = sd->accel_internal[0];
    out[4] = sd->accel_external[0];
    out[5] = sd->net_traffic;
    out[6] = (float)sd->rf_nrf_dbm;
    out[7] = sd->temp;
}
