/**
 * @file    bm_spi.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm_dma.h"
#include "bm.h"
#include <string.h>

static uint8_t s_spi_dummy_tx[64];
static uint8_t s_spi_dummy_rx[64];

typedef struct {
    bm_dma_ch_t ch_tx;
    bm_dma_ch_t ch_rx;
    uint32_t req_tx;
    uint32_t req_rx;
} bm_spi_dma_map_t;

static const bm_spi_dma_map_t s_spi1_dma = {
    BM_DMA_CH_SPI1_TX, BM_DMA_CH_SPI1_RX,
    BM_DMAMUX_REQ_SPI1_TX, BM_DMAMUX_REQ_SPI1_RX
};

static const bm_spi_dma_map_t s_spi2_dma = {
    BM_DMA_CH_SPI2_TX, BM_DMA_CH_SPI2_RX,
    BM_DMAMUX_REQ_SPI2_TX, BM_DMAMUX_REQ_SPI2_RX
};

static const bm_spi_dma_map_t *spi_dma_map(SPI_TypeDef *spi)
{
    if (spi == SPI1) {
        return &s_spi1_dma;
    }
    if (spi == SPI2) {
        return &s_spi2_dma;
    }
    return NULL;
}

/**
 * @brief   TODO: describe what spi1_hw_init() does
 */
static void spi1_hw_init(void)
{
    const uint32_t prio_group = 3U;

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    GPIOA->MODER = (GPIOA->MODER & ~((3U << 2) | (3U << 12) | (3U << 14)))
                 | (2U << 2) | (2U << 12) | (2U << 14);
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~0xF0000F0CU) | 0x5000500U;

    NVIC_SetPriority(SPI1_IRQn, NVIC_EncodePriority(prio_group, 13U, 0));
    NVIC_EnableIRQ(SPI1_IRQn);
}

/**
 * @brief   TODO: describe what spi2_hw_init() does
 */
static void spi2_hw_init(void)
{
    const uint32_t prio_group = 3U;

    RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN;
    GPIOC->MODER = (GPIOC->MODER & ~((3U << 2) | (3U << 4))) | (2U << 2) | (2U << 4);
    GPIOC->AFR[0] = (GPIOC->AFR[0] & ~0xFFU) | 0x35U;
    GPIOA->MODER = (GPIOA->MODER & ~(3U << 18)) | (2U << 18);
    GPIOA->AFR[1] = (GPIOA->AFR[1] & ~(0xFU << 4)) | (0x5U << 4);

    NVIC_SetPriority(SPI2_IRQn, NVIC_EncodePriority(prio_group, 13U, 0));
    NVIC_EnableIRQ(SPI2_IRQn);
}

/**
 * @brief   TODO: describe what spi_configure() does
 * @param   spi  TODO: describe parameter
 */
static void spi_configure(SPI_TypeDef *spi)
{
    spi->CR1 = 0;
    spi->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI
             | (3U << SPI_CR1_BR_Pos);
    spi->CR1 |= SPI_CR1_SPE;
}

/**
 * @brief   TODO: describe what bm_spi1_init() does
 */
void bm_spi1_init(void)
{
    spi1_hw_init();
    spi_configure(SPI1);
}

/**
 * @brief   TODO: describe what bm_spi2_init() does
 */
void bm_spi2_init(void)
{
    spi2_hw_init();
    spi_configure(SPI2);
}

/**
 * @brief   TODO: describe what bm_spi1_irq_handler() does
 */
void bm_spi1_irq_handler(void)
{
    if (SPI1->SR & SPI_SR_OVR) {
        (void)SPI1->DR;
        (void)SPI1->SR;
    }
}

/**
 * @brief   TODO: describe what bm_spi2_irq_handler() does
 */
void bm_spi2_irq_handler(void)
{
    if (SPI2->SR & SPI_SR_OVR) {
        (void)SPI2->DR;
        (void)SPI2->SR;
    }
}

/**
 * @brief   TODO: describe what spi_dma_xfer() does
 * @param   spi  TODO: describe parameter
 * @param   map  TODO: describe parameter
 * @param   tx  TODO: describe parameter
 * @param   rx  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static bool spi_dma_xfer(SPI_TypeDef *spi, const bm_spi_dma_map_t *map,
                         const uint8_t *tx, uint8_t *rx, uint16_t len,
                         uint32_t timeout_ms)
{
    if (!map || len == 0U || len > sizeof(s_spi_dummy_tx)) {
        return false;
    }

    uint8_t *rx_buf = rx;
    const uint8_t *tx_buf = tx;

    if (!rx_buf) {
        rx_buf = s_spi_dummy_rx;
    }
    if (!tx_buf) {
        memset(s_spi_dummy_tx, 0xFF, len);
        tx_buf = s_spi_dummy_tx;
    }

    uint32_t dr = (uint32_t)&spi->DR;

    bm_dma_ch_clear_tc(map->ch_rx);
    bm_dma_ch_clear_tc(map->ch_tx);

    if (!bm_dma_ch_start(map->ch_rx, map->req_rx, dr, (uint32_t)rx_buf, len,
                         true, true, false, 0U)) {
        return false;
    }
    if (!bm_dma_ch_start(map->ch_tx, map->req_tx, dr, (uint32_t)tx_buf, len,
                         false, true, false, 0U)) {
        bm_dma_ch_stop(map->ch_rx);
        return false;
    }

    spi->CR2 |= SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN;

    bool ok_rx = bm_dma_ch_wait_tc(map->ch_rx, timeout_ms);
    bool ok_tx = bm_dma_ch_wait_tc(map->ch_tx, timeout_ms);

    spi->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

    return ok_rx && ok_tx;
}

/**
 * @brief   TODO: describe what bm_spi_transmit() does
 * @param   spi  TODO: describe parameter
 * @param   tx  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_spi_transmit(SPI_TypeDef *spi, const uint8_t *tx, uint16_t len, uint32_t timeout_ms)
{
    return spi_dma_xfer(spi, spi_dma_map(spi), tx, NULL, len, timeout_ms);
}

/**
 * @brief   TODO: describe what bm_spi_receive() does
 * @param   spi  TODO: describe parameter
 * @param   rx  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_spi_receive(SPI_TypeDef *spi, uint8_t *rx, uint16_t len, uint32_t timeout_ms)
{
    return spi_dma_xfer(spi, spi_dma_map(spi), NULL, rx, len, timeout_ms);
}

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
                             uint16_t len, uint32_t timeout_ms)
{
    return spi_dma_xfer(spi, spi_dma_map(spi), tx, rx, len, timeout_ms);
}
