/**
 * @file    bm_init.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm.h"

/**
 * @brief   TODO: describe what bm_system_init() does
 */
void bm_system_init(void)
{
    bm_board_init();
    bm_rcc_init();
    bm_dma_init();
    bm_gpio_init();
    bm_i2c1_init();
    bm_i2c3_init();
    bm_lpuart1_init();
    bm_spi1_init();
    bm_spi2_init();
    bm_aes1_init();
    bm_crc_init();
    bm_rng_init();
    bm_pka_init();
    bm_adc1_init();
    bm_tim1_tick_init(15U);
}
