/**
 * @file    bm_board.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm.h"

/**
 * @brief   TODO: describe what bm_board_init() does
 */
void bm_board_init(void)
{
    const uint32_t prio_group = 3U;

    NVIC_SetPriorityGrouping(prio_group);

    NVIC_SetPriority(MemoryManagement_IRQn, NVIC_EncodePriority(prio_group, 10, 0));
    NVIC_SetPriority(BusFault_IRQn, NVIC_EncodePriority(prio_group, 10, 0));
    NVIC_SetPriority(UsageFault_IRQn, NVIC_EncodePriority(prio_group, 10, 0));
    NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(prio_group, 10, 0));
    NVIC_SetPriority(DebugMonitor_IRQn, NVIC_EncodePriority(prio_group, 10, 0));
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(prio_group, 15, 0));

    NVIC_SetPriority(FLASH_IRQn, NVIC_EncodePriority(prio_group, 10, 0));
    NVIC_EnableIRQ(FLASH_IRQn);
    NVIC_SetPriority(RCC_IRQn, NVIC_EncodePriority(prio_group, 10, 0));
    NVIC_EnableIRQ(RCC_IRQn);
    NVIC_SetPriority(C2SEV_PWR_C2H_IRQn, NVIC_EncodePriority(prio_group, 10, 0));
    NVIC_EnableIRQ(C2SEV_PWR_C2H_IRQn);
    NVIC_SetPriority(PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQn, NVIC_EncodePriority(prio_group, 13, 0));
    NVIC_EnableIRQ(PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQn);
    NVIC_SetPriority(FPU_IRQn, NVIC_EncodePriority(prio_group, 10, 0));
    NVIC_EnableIRQ(FPU_IRQn);
}
