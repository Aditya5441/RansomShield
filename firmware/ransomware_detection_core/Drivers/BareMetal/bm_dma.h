/**
 * @file    bm_dma.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef BM_DMA_H
#define BM_DMA_H

#include "stm32wbxx.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BM_DMA_CH_ADC = 0,
    BM_DMA_CH_SPI1_TX,
    BM_DMA_CH_SPI1_RX,
    BM_DMA_CH_SPI2_TX,
    BM_DMA_CH_SPI2_RX,
    BM_DMA_CH_LPUART1_TX,
    BM_DMA_CH_COUNT
} bm_dma_ch_t;

void bm_dma_init(void);
void bm_dma_ch_irq_handler(bm_dma_ch_t ch);

bool bm_dma_ch_start(bm_dma_ch_t ch, uint32_t mux_req_id,
                     uint32_t periph_addr, uint32_t mem_addr, uint16_t count,
                     bool periph_to_mem, bool mem_inc, bool circ,
                     uint8_t data_width_log2);

void bm_dma_ch_stop(bm_dma_ch_t ch);
bool bm_dma_ch_wait_tc(bm_dma_ch_t ch, uint32_t timeout_ms);
void bm_dma_ch_clear_tc(bm_dma_ch_t ch);

#define BM_DMAMUX_REQ_ADC1        5U
#define BM_DMAMUX_REQ_SPI1_RX     6U
#define BM_DMAMUX_REQ_SPI1_TX     7U
#define BM_DMAMUX_REQ_SPI2_RX     8U
#define BM_DMAMUX_REQ_SPI2_TX     9U
#define BM_DMAMUX_REQ_LPUART1_TX  17U

/**
 * @brief   TODO: describe what bm_dmamux1_ovr_irq_handler() does
 */
void bm_dmamux1_ovr_irq_handler(void);

#endif
