/**
 * @file    stm32wbxx_it.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "main.h"
#include "stm32wbxx_it.h"
#include "bm.h"
#include "bm_dma.h"

/**
 * @brief   TODO: describe what NMI_Handler() does
 */
void NMI_Handler(void)
{
  while (1) {
  }
}

/**
 * @brief   TODO: describe what HardFault_Handler() does
 */
void HardFault_Handler(void)
{
  while (1) {
  }
}

/**
 * @brief   TODO: describe what MemManage_Handler() does
 */
void MemManage_Handler(void)
{
  while (1) {
  }
}

/**
 * @brief   TODO: describe what BusFault_Handler() does
 */
void BusFault_Handler(void)
{
  while (1) {
  }
}

/**
 * @brief   TODO: describe what UsageFault_Handler() does
 */
void UsageFault_Handler(void)
{
  while (1) {
  }
}

/**
 * @brief   TODO: describe what DebugMon_Handler() does
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief   TODO: describe what TIM1_UP_TIM16_IRQHandler() does
 */
void TIM1_UP_TIM16_IRQHandler(void)
{
  bm_tim1_irq_handler();
}

/**
 * @brief   TODO: describe what FLASH_IRQHandler() does
 */
void FLASH_IRQHandler(void)
{
}

/**
 * @brief   TODO: describe what RCC_IRQHandler() does
 */
void RCC_IRQHandler(void)
{
}

/**
 * @brief   TODO: describe what DMA1_Channel1_IRQHandler() does
 */
void DMA1_Channel1_IRQHandler(void)
{
  bm_dma_ch_irq_handler(BM_DMA_CH_ADC);
}

/**
 * @brief   TODO: describe what DMA1_Channel2_IRQHandler() does
 */
void DMA1_Channel2_IRQHandler(void)
{
  bm_dma_ch_irq_handler(BM_DMA_CH_SPI1_TX);
}

/**
 * @brief   TODO: describe what DMA1_Channel3_IRQHandler() does
 */
void DMA1_Channel3_IRQHandler(void)
{
  bm_dma_ch_irq_handler(BM_DMA_CH_SPI1_RX);
}

/**
 * @brief   TODO: describe what DMA1_Channel4_IRQHandler() does
 */
void DMA1_Channel4_IRQHandler(void)
{
  bm_dma_ch_irq_handler(BM_DMA_CH_SPI2_TX);
}

/**
 * @brief   TODO: describe what DMA1_Channel5_IRQHandler() does
 */
void DMA1_Channel5_IRQHandler(void)
{
  bm_dma_ch_irq_handler(BM_DMA_CH_SPI2_RX);
}

/**
 * @brief   TODO: describe what DMA1_Channel6_IRQHandler() does
 */
void DMA1_Channel6_IRQHandler(void)
{
  bm_dma_ch_irq_handler(BM_DMA_CH_LPUART1_TX);
}

/**
 * @brief   TODO: describe what ADC1_IRQHandler() does
 */
void ADC1_IRQHandler(void)
{
  bm_adc1_irq_handler();
}

/**
 * @brief   TODO: describe what C2SEV_PWR_C2H_IRQHandler() does
 */
void C2SEV_PWR_C2H_IRQHandler(void)
{
}

/**
 * @brief   TODO: describe what I2C1_EV_IRQHandler() does
 */
void I2C1_EV_IRQHandler(void)
{
  bm_i2c1_ev_irq_handler();
}

/**
 * @brief   TODO: describe what I2C1_ER_IRQHandler() does
 */
void I2C1_ER_IRQHandler(void)
{
  bm_i2c1_er_irq_handler();
}

/**
 * @brief   TODO: describe what I2C3_EV_IRQHandler() does
 */
void I2C3_EV_IRQHandler(void)
{
  bm_i2c3_ev_irq_handler();
}

/**
 * @brief   TODO: describe what I2C3_ER_IRQHandler() does
 */
void I2C3_ER_IRQHandler(void)
{
  bm_i2c3_er_irq_handler();
}

/**
 * @brief   TODO: describe what SPI1_IRQHandler() does
 */
void SPI1_IRQHandler(void)
{
  bm_spi1_irq_handler();
}

/**
 * @brief   TODO: describe what SPI2_IRQHandler() does
 */
void SPI2_IRQHandler(void)
{
  bm_spi2_irq_handler();
}

/**
 * @brief   TODO: describe what LPUART1_IRQHandler() does
 */
void LPUART1_IRQHandler(void)
{
  bm_lpuart1_irq_handler();
}

/**
 * @brief   TODO: describe what PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQHandler() does
 */
void PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQHandler(void)
{
}

/**
 * @brief   TODO: describe what AES1_IRQHandler() does
 */
void AES1_IRQHandler(void)
{
  bm_aes1_irq_handler();
}

/**
 * @brief   TODO: describe what RNG_IRQHandler() does
 */
void RNG_IRQHandler(void)
{
  bm_rng_irq_handler();
}

/**
 * @brief   TODO: describe what FPU_IRQHandler() does
 */
void FPU_IRQHandler(void)
{
}

/**
 * @brief   TODO: describe what DMAMUX1_OVR_IRQHandler() does
 */
void DMAMUX1_OVR_IRQHandler(void)
{
  bm_dmamux1_ovr_irq_handler();
}

/**
 * @brief   TODO: describe what PKA_IRQHandler() does
 */
void PKA_IRQHandler(void)
{
}
