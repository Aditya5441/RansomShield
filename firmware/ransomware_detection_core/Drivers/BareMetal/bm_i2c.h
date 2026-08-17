/**
 * @file    bm_i2c.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef BM_I2C_H
#define BM_I2C_H

#include "stm32wbxx.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   TODO: describe what bm_i2c1_init() does
 */
void bm_i2c1_init(void);

/**
 * @brief   TODO: describe what bm_i2c3_init() does
 */
void bm_i2c3_init(void);

/**
 * @brief   TODO: describe what bm_i2c_master_transmit() does
 * @param   i2c  TODO: describe parameter
 * @param   dev_addr_7bit  TODO: describe parameter
 * @param   data  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_i2c_master_transmit(I2C_TypeDef *i2c, uint16_t dev_addr_7bit,
                            const uint8_t *data, uint16_t len, uint32_t timeout_ms);

/**
 * @brief   TODO: describe what bm_i2c_master_receive() does
 * @param   i2c  TODO: describe parameter
 * @param   dev_addr_7bit  TODO: describe parameter
 * @param   data  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_i2c_master_receive(I2C_TypeDef *i2c, uint16_t dev_addr_7bit,
                           uint8_t *data, uint16_t len, uint32_t timeout_ms);

/**
 * @brief   TODO: describe what bm_i2c1_ev_irq_handler() does
 */
void bm_i2c1_ev_irq_handler(void);

/**
 * @brief   TODO: describe what bm_i2c1_er_irq_handler() does
 */
void bm_i2c1_er_irq_handler(void);

/**
 * @brief   TODO: describe what bm_i2c3_ev_irq_handler() does
 */
void bm_i2c3_ev_irq_handler(void);

/**
 * @brief   TODO: describe what bm_i2c3_er_irq_handler() does
 */
void bm_i2c3_er_irq_handler(void);

#endif
