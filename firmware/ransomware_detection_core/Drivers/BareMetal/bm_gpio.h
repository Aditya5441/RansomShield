/**
 * @file    bm_gpio.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef BM_GPIO_H
#define BM_GPIO_H

#include "stm32wbxx.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   TODO: describe what bm_gpio_init() does
 */
void bm_gpio_init(void);

/**
 * @brief   TODO: describe what bm_gpio_write() does
 * @param   port  TODO: describe parameter
 * @param   pin  TODO: describe parameter
 * @param   high  TODO: describe parameter
 */
void bm_gpio_write(GPIO_TypeDef *port, uint16_t pin, bool high);

#endif
