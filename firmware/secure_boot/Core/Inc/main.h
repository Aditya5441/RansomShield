/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c - baremetal (no HAL) build.
  ******************************************************************************
  * Register-level access only: this project talks to peripherals directly
  * through the CMSIS device header (stm32wb55xx.h) instead of ST's HAL.
  ******************************************************************************
  */
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbxx.h"     /* CMSIS device header: register/struct definitions only */
#include <stdint.h>

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void SystemClock_Config(void);
void USART1_Init(void);
void GPIO_Init(void);

/* USART1 exported for use by other modules if needed */
#define DEBUG_USART   USART1

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
