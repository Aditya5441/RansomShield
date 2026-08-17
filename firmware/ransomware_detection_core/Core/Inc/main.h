/**
 * @file    main.h
 * @brief   TODO: one-line description of this file's purpose
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32wbxx.h"
#include "bm.h"

void Error_Handler(void);
void MX_AppFreeRTOS_Init(void);

#ifdef __cplusplus
}
#endif

#endif
