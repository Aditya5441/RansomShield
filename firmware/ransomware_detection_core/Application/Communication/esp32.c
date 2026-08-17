/**
 * @file    esp32.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "esp32.h"
#include "bm.h"

/**
 * @brief   TODO: describe what ESP_Send() does
 * @param   data  TODO: describe parameter
 * @param   len  TODO: describe parameter
 */
void ESP_Send(uint8_t *data, uint16_t len)
{
    bm_lpuart_transmit(data, len, 100);
}
