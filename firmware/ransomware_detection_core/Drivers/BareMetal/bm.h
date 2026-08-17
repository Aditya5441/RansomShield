/**
 * @file    bm.h
 * @brief   Umbrella header for the bare-metal driver layer; includes every
 *          per-peripheral driver header (bm_gpio.h, bm_i2c.h, bm_spi.h, etc.)
 */

#ifndef BM_H
#define BM_H

#include "bm_board.h"
#include "bm_rcc.h"
#include "bm_init.h"
#include "bm_tim.h"
#include "bm_gpio.h"
#include "bm_i2c.h"
#include "bm_spi.h"
#include "bm_uart.h"
#include "bm_adc.h"
#include "bm_rng.h"
#include "bm_aes.h"
#include "bm_crc_pka.h"
#include "bm_dma.h"

#endif
