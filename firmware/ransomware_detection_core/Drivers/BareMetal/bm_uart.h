/**
 * @file    bm_uart.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef BM_UART_H
#define BM_UART_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   TODO: describe what bm_lpuart1_init() does
 */
void bm_lpuart1_init(void);

/**
 * @brief   TODO: describe what bm_lpuart_transmit() does
 * @param   data  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_lpuart_transmit(const uint8_t *data, uint16_t len, uint32_t timeout_ms);

/**
 * @brief   TODO: describe what bm_lpuart1_irq_handler() does
 */
void bm_lpuart1_irq_handler(void);

#endif
