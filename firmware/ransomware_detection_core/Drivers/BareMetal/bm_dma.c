/**
 * @file    bm_dma.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm_dma.h"
#include "bm.h"

#define BM_DMA_PRIO_LOW   (1U << DMA_CCR_PL_Pos)

typedef struct {
    DMA_Channel_TypeDef *ch;
    DMAMUX_Channel_TypeDef *mux;
    IRQn_Type irq;
    volatile bool tc_done;
    volatile bool te_error;
} bm_dma_ctx_t;

static bm_dma_ctx_t s_dma[BM_DMA_CH_COUNT] = {
    { DMA1_Channel1, DMAMUX1_Channel0, DMA1_Channel1_IRQn },
    { DMA1_Channel2, DMAMUX1_Channel1, DMA1_Channel2_IRQn },
    { DMA1_Channel3, DMAMUX1_Channel2, DMA1_Channel3_IRQn },
    { DMA1_Channel4, DMAMUX1_Channel3, DMA1_Channel4_IRQn },
    { DMA1_Channel5, DMAMUX1_Channel4, DMA1_Channel5_IRQn },
    { DMA1_Channel6, DMAMUX1_Channel5, DMA1_Channel6_IRQn },
};

/**
 * @brief   TODO: describe what dma_tc_flag() does
 * @param   ch  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static uint32_t dma_tc_flag(bm_dma_ch_t ch)
{
    return (1U << (1U + (4U * (uint32_t)ch)));
}

/**
 * @brief   TODO: describe what dma_te_flag() does
 * @param   ch  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static uint32_t dma_te_flag(bm_dma_ch_t ch)
{
    return (1U << (3U + (4U * (uint32_t)ch)));
}

/**
 * @brief   TODO: describe what bm_dma_init() does
 */
void bm_dma_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN;

    const uint32_t prio_group = 3U;
    const uint32_t dma_prio = 10U;

    for (uint32_t i = 0; i < BM_DMA_CH_COUNT; i++) {
        NVIC_SetPriority(s_dma[i].irq, NVIC_EncodePriority(prio_group, dma_prio, 0));
        NVIC_EnableIRQ(s_dma[i].irq);
    }

    NVIC_SetPriority(DMAMUX1_OVR_IRQn, NVIC_EncodePriority(prio_group, dma_prio, 0));
    NVIC_EnableIRQ(DMAMUX1_OVR_IRQn);
}

/**
 * @brief   TODO: describe what bm_dma_ch_irq_handler() does
 * @param   ch  TODO: describe parameter
 */
void bm_dma_ch_irq_handler(bm_dma_ch_t ch)
{
    if (ch >= BM_DMA_CH_COUNT) {
        return;
    }

    uint32_t tc = dma_tc_flag(ch);
    uint32_t te = dma_te_flag(ch);

    if (DMA1->ISR & te) {
        DMA1->IFCR = te;
        s_dma[ch].te_error = true;
        s_dma[ch].tc_done = true;
    }
    if (DMA1->ISR & tc) {
        DMA1->IFCR = tc;
        s_dma[ch].tc_done = true;
    }
}

/**
 * @brief   TODO: describe what bm_dma_ch_stop() does
 * @param   ch  TODO: describe parameter
 */
void bm_dma_ch_stop(bm_dma_ch_t ch)
{
    if (ch >= BM_DMA_CH_COUNT) {
        return;
    }
    s_dma[ch].ch->CCR &= ~DMA_CCR_EN;
    s_dma[ch].tc_done = false;
    s_dma[ch].te_error = false;
}

/**
 * @brief   TODO: describe what bm_dma_ch_start() does
 * @param   ch  TODO: describe parameter
 * @param   mux_req_id  TODO: describe parameter
 * @param   periph_addr  TODO: describe parameter
 * @param   mem_addr  TODO: describe parameter
 * @param   count  TODO: describe parameter
 * @param   periph_to_mem  TODO: describe parameter
 * @param   mem_inc  TODO: describe parameter
 * @param   circ  TODO: describe parameter
 * @param   data_width_log2  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_dma_ch_start(bm_dma_ch_t ch, uint32_t mux_req_id,
                     uint32_t periph_addr, uint32_t mem_addr, uint16_t count,
                     bool periph_to_mem, bool mem_inc, bool circ,
                     uint8_t data_width_log2)
{
    if (ch >= BM_DMA_CH_COUNT || count == 0U) {
        return false;
    }

    bm_dma_ctx_t *ctx = &s_dma[ch];
    DMA_Channel_TypeDef *dma = ctx->ch;

    bm_dma_ch_stop(ch);

    ctx->mux->CCR = (ctx->mux->CCR & ~DMAMUX_CxCR_DMAREQ_ID) | (mux_req_id & DMAMUX_CxCR_DMAREQ_ID);

    dma->CPAR = periph_addr;
    dma->CMAR = mem_addr;
    dma->CNDTR = count;

    uint32_t ccr = BM_DMA_PRIO_LOW | DMA_CCR_TCIE | DMA_CCR_TEIE;
    if (periph_to_mem) {
        ccr &= ~DMA_CCR_DIR;
    } else {
        ccr |= DMA_CCR_DIR;
    }
    if (mem_inc) {
        ccr |= DMA_CCR_MINC;
    }
    if (circ) {
        ccr |= DMA_CCR_CIRC;
    }

    if (data_width_log2 == 1U) {
        ccr |= DMA_CCR_PSIZE_0 | DMA_CCR_MSIZE_0;
    } else if (data_width_log2 >= 2U) {
        ccr |= DMA_CCR_PSIZE_1 | DMA_CCR_MSIZE_1;
    }

    dma->CCR = ccr;
    ctx->tc_done = false;
    ctx->te_error = false;
    dma->CCR |= DMA_CCR_EN;
    return true;
}

/**
 * @brief   TODO: describe what bm_dma_ch_wait_tc() does
 * @param   ch  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_dma_ch_wait_tc(bm_dma_ch_t ch, uint32_t timeout_ms)
{
    if (ch >= BM_DMA_CH_COUNT) {
        return false;
    }

    uint32_t deadline = bm_tick_get() + timeout_ms;
    while (!s_dma[ch].tc_done) {
        if (bm_tick_get() >= deadline) {
            bm_dma_ch_stop(ch);
            return false;
        }
    }
    bool ok = !s_dma[ch].te_error;
    if (!((s_dma[ch].ch->CCR & DMA_CCR_CIRC) != 0U)) {
        bm_dma_ch_stop(ch);
    }
    return ok;
}

/**
 * @brief   TODO: describe what bm_dma_ch_clear_tc() does
 * @param   ch  TODO: describe parameter
 */
void bm_dma_ch_clear_tc(bm_dma_ch_t ch)
{
    if (ch < BM_DMA_CH_COUNT) {
        s_dma[ch].tc_done = false;
        s_dma[ch].te_error = false;
    }
}

/**
 * @brief   TODO: describe what bm_dmamux1_ovr_irq_handler() does
 */
void bm_dmamux1_ovr_irq_handler(void)
{
    for (uint32_t i = 0; i < BM_DMA_CH_COUNT; i++) {
        if (DMAMUX1_ChannelStatus->CSR & (1UL << i)) {
            DMAMUX1_ChannelStatus->CFR = (1UL << i);
        }
        s_dma[i].te_error = true;
        s_dma[i].tc_done = true;
    }
}
