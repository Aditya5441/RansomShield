/**
 * @file    bm_adc.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm_dma.h"
#include "bm.h"

#define BM_ADC_DMA_SAMPLES  16U

static uint16_t s_adc_dma_buf[BM_ADC_DMA_SAMPLES];

/**
 * @brief   TODO: describe what bm_adc1_init() does
 */
void bm_adc1_init(void)
{
    const uint32_t prio_group = 3U;

    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_ADCSEL) | RCC_CCIPR_ADCSEL_1;
    RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;

    GPIOC->MODER = (GPIOC->MODER & ~(3U << 6)) | (3U << 6);
    GPIOA->MODER = (GPIOA->MODER & ~(3U << 8)) | (3U << 8);

    ADC1->CR = ADC_CR_ADVREGEN;
    for (volatile uint32_t d = 0; d < 10000U; d++) {
    }

    ADC1->CFGR = ADC_CFGR_CONT | ADC_CFGR_OVRMOD | ADC_CFGR_DMAEN | ADC_CFGR_DMACFG;
    ADC1->SMPR1 = (2U << ADC_SMPR1_SMP4_Pos);
    ADC1->SQR1 = (4U << ADC_SQR1_SQ1_Pos) | (0U << ADC_SQR1_L_Pos);

    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {
    }

    bm_dma_ch_start(BM_DMA_CH_ADC, BM_DMAMUX_REQ_ADC1,
                    (uint32_t)&ADC1->DR, (uint32_t)s_adc_dma_buf,
                    BM_ADC_DMA_SAMPLES, true, true, true, 1U);

    ADC1->CR |= ADC_CR_ADSTART;

    NVIC_SetPriority(ADC1_IRQn, NVIC_EncodePriority(prio_group, 10U, 0));
    NVIC_EnableIRQ(ADC1_IRQn);
}

/**
 * @brief   TODO: describe what bm_adc1_irq_handler() does
 */
void bm_adc1_irq_handler(void)
{
    if (ADC1->ISR & ADC_ISR_OVR) {
        ADC1->ISR = ADC_ISR_OVR;
    }
}

/**
 * @brief   TODO: describe what bm_adc_read_channel() does
 * @param   channel  TODO: describe parameter
 * @param   value  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_adc_read_channel(uint32_t channel, uint16_t *value, uint32_t timeout_ms)
{
    (void)timeout_ms;

    if (!value) {
        return false;
    }

    if (channel != 4U) {
        ADC1->SQR1 = (ADC1->SQR1 & ~ADC_SQR1_SQ1) | (channel << ADC_SQR1_SQ1_Pos);
        ADC1->CR |= ADC_CR_ADSTART;
    }

    (void)timeout_ms;
    *value = s_adc_dma_buf[BM_ADC_DMA_SAMPLES - 1U];
    return true;
}
