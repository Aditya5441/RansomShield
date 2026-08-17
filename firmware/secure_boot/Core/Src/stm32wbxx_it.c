/**
  ******************************************************************************
  * @file    stm32wbxx_it.c
  * @brief   Interrupt Service Routines - baremetal build.
  *
  * SysTick_Handler() is defined in main.c (it just increments a free-running
  * millisecond counter there instead of calling HAL_IncTick()). Every other
  * handler here is unchanged from the original - none of them actually
  * called into HAL.
  ******************************************************************************
  */
#include "main.h"
#include "stm32wbxx_it.h"

void NMI_Handler(void)
{
    while (1)
    {
    }
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

/******************************************************************************/
/* STM32WBxx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for any peripherals this stage starts      */
/* using (see the startup file startup_stm32wb55rgvx.s for the full list).    */
/******************************************************************************/
