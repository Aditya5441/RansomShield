/**
 * @file    bm_gpio.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm.h"

/**
 * @brief   TODO: describe what bm_gpio_init() does
 */
void bm_gpio_init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN;

    GPIOB->MODER = (GPIOB->MODER & ~((3U << 12) | (3U << 14) | (3U << 16)))
                 | (1U << 12) | (1U << 14) | (1U << 16);
    GPIOB->BSRR = GPIO_BSRR_BS6 | GPIO_BSRR_BS7 | GPIO_BSRR_BS8;
}

/**
 * @brief   TODO: describe what bm_gpio_write() does
 * @param   port  TODO: describe parameter
 * @param   pin  TODO: describe parameter
 * @param   high  TODO: describe parameter
 */
void bm_gpio_write(GPIO_TypeDef *port, uint16_t pin, bool high)
{
    if (high) {
        port->BSRR = pin;
    } else {
        port->BSRR = (uint32_t)pin << 16U;
    }
}
