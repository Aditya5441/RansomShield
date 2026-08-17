/* bootloader.c */
#include "bootloader.h"
#include "main.h"
#include "crypto.h"
#include "secure_vault.h"
#include "secure_boot.h"

static uint8_t BL_IsValidApp(void);
static void BL_JumpToApp(void);

void Bootloader_Run(void)
{
    uint8_t key[32];

    SV_GetKey(key);

    /* Keep key derivation here if you need it later for image verification.
       Do not decrypt dummy/uninitialized data. */

    /* Two independent gates before jumping to the app:
     *   1. BL_IsValidApp()          - cheap plausibility check (SP/reset
     *                                  vector land inside SRAM/flash).
     *   2. SecureBoot_VerifyFirmware() - the real gate: magic + size +
     *                                  SHA-256 + ECDSA signature +
     *                                  anti-rollback, mirroring what
     *                                  Secure Boot itself does for this
     *                                  bootloader's own image.
     * Previously only (1) ran, so any image with a plausible-looking
     * vector table - signed or not - would boot. */
    if (BL_IsValidApp() && (SecureBoot_VerifyFirmware() == 0))
    {
        BL_JumpToApp();
    }

    while (1)
    {
        /* Stay in bootloader / recovery mode */
    }
}

static uint8_t BL_IsValidApp(void)
{
    /* Vector table lives after the SecureBoot header, at APP_CODE_ADDR -
     * not at APP_ADDR, which is the header's magic/size/version bytes. */
    uint32_t app_sp = *(volatile uint32_t *)APP_CODE_ADDR;
    uint32_t app_reset = *(volatile uint32_t *)(APP_CODE_ADDR + 4U);

    if ((app_sp < SRAM_START_ADDR) || (app_sp > SRAM_END_ADDR))
    {
        return 0U;
    }

    if ((app_reset < APP_CODE_ADDR) || (app_reset > FLASH_END_ADDR))
    {
        return 0U;
    }

    if ((app_sp == 0xFFFFFFFFUL) || (app_reset == 0xFFFFFFFFUL))
    {
        return 0U;
    }

    return 1U;
}

static void BL_JumpToApp(void)
{
    uint32_t app_sp = *(volatile uint32_t *)APP_CODE_ADDR;
    uint32_t app_reset = *(volatile uint32_t *)(APP_CODE_ADDR + 4U);
    void (*App_ResetHandler)(void) = (void (*)(void))app_reset;

    HAL_USART_DeInit(&husart1);
    HAL_RCC_DeInit();
    HAL_DeInit();

    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL  = 0U;

    __disable_irq();
    SCB->VTOR = APP_CODE_ADDR;
    __set_MSP(app_sp);
    __DSB();
    __ISB();
    __enable_irq();

    App_ResetHandler();
}

