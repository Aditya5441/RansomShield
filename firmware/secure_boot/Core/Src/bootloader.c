#include "bootloader.h"
#include "secure_boot.h"
#include "main.h"

#define FW_START_ADDR   0x08008000UL   /* keep in sync with secure_boot.c */

void Bootloader_Run(void)
{
    int rc = SecureBoot_VerifyFirmware();

    if (rc != 0)
    {
        /* Verification failed (bad magic/size/signature/rollback, or
         * RDP0-unsigned): drop into the ST system bootloader for recovery
         * rather than executing unverified code. */
        SecureBoot_JumpToBootloader();
        /* SecureBoot_JumpToBootloader() never returns. */
        while (1) { }
    }

    /* Verified OK: hand control to the Bootloader image. Its vector table
     * lives at FW_START_ADDR (MSP at +0, Reset_Handler at +4), matching
     * how STM32 images self-relocate. */
    typedef void (*reset_handler_t)(void);
    uint32_t *vector_table = (uint32_t *)FW_START_ADDR;

    __disable_irq();
    SysTick->CTRL = 0;

    /* Relocate the vector table and set the stack pointer before jumping,
     * so the next stage's own exception handlers are live immediately. */
    SCB->VTOR = FW_START_ADDR;
    __set_MSP(vector_table[0]);

    reset_handler_t next_reset_handler = (reset_handler_t)vector_table[1];
    next_reset_handler();

    while (1) { }   /* unreachable */
}
