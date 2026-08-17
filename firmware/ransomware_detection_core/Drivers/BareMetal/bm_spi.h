/**
 * @file    bm_spi.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef BM_SPI_H
#define BM_SPI_H

#include "stm32wbxx.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   TODO: describe what bm_spi1_init() does
 */
void bm_spi1_init(void);

/**
 * @brief   TODO: describe what bm_spi2_init() does
 */
void bm_spi2_init(void);

/**
 * @brief   TODO: describe what bm_spi_transmit() does
 * @param   spi  TODO: describe parameter
 * @param   tx  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_spi_transmit(SPI_TypeDef *spi, const uint8_t *tx, uint16_t len, uint32_t timeout_ms);

/**
 * @brief   TODO: describe what bm_spi_receive() does
 * @param   spi  TODO: describe parameter
 * @param   rx  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_spi_receive(SPI_TypeDef *spi, uint8_t *rx, uint16_t len, uint32_t timeout_ms);

/**
 * @brief   TODO: describe what bm_spi_transmit_receive() does
 * @param   spi  TODO: describe parameter
 * @param   tx  TODO: describe parameter
 * @param   rx  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_spi_transmit_receive(SPI_TypeDef *spi, const uint8_t *tx, uint8_t *rx,
                             uint16_t len, uint32_t timeout_ms);

/**
 * @brief   TODO: describe what bm_spi1_irq_handler() does
 */
void bm_spi1_irq_handler(void);

/**
 * @brief   TODO: describe what bm_spi2_irq_handler() does
 */
void bm_spi2_irq_handler(void);

#endif
