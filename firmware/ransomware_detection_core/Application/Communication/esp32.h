/**
 * @file    esp32.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef APPLICATION_COMM_ESP32_H_
#define APPLICATION_COMM_ESP32_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ESP_Send(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
