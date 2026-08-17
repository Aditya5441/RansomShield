/**
 * @file    stm32wbxx_it.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef __STM32WBxx_IT_H
#define __STM32WBxx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);
void FLASH_IRQHandler(void);
void RCC_IRQHandler(void);
void DMA1_Channel1_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);
void DMA1_Channel3_IRQHandler(void);
void DMA1_Channel4_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);
void DMA1_Channel6_IRQHandler(void);
void ADC1_IRQHandler(void);
void C2SEV_PWR_C2H_IRQHandler(void);
void TIM1_UP_TIM16_IRQHandler(void);
void PKA_IRQHandler(void);
void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void I2C3_EV_IRQHandler(void);
void I2C3_ER_IRQHandler(void);
void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
void LPUART1_IRQHandler(void);
void PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQHandler(void);
void AES1_IRQHandler(void);
void RNG_IRQHandler(void);
void FPU_IRQHandler(void);
void DMAMUX1_OVR_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif
