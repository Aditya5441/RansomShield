#include <string.h>
#include "secure_boot.h"
#include "crypto.h"
#include "ecdsa_verify.h"
#include "main.h"
#include "stm32wbxx.h"

/* Secure Boot (this stage) verifies the NEXT stage in the chain, which is
 * the Bootloader image. Bootloader_DSP's own linker script places its
 * FLASH region at ORIGIN = 0x08008000, LENGTH = 32K (see
 * STM32WB55RGVX_FLASH.ld in that project) - these two constants must stay
 * in sync with that file or verification will read the wrong slot. */
#define FW_START_ADDR      0x08008000UL  // Start of the Bootloader image
#define FW_MAX_SIZE        0x8000UL      // 32K Bootloader slot
#define SIGNATURE_SIZE     ECDSA_SIGNATURE_LEN  // Real P-256 sig = 64 bytes
#define PUBKEY_ADDR        0x08800000UL  /* Flash sector reserved for the
 * provisioned public key; populated by a separate flashing step
 * (tools/keygen.py -> pubkey.bin), ideally write-protected afterward. */

#define OB_RDP_LEVEL_0      0xAAU

/* TAMP backup registers survive resets/power-cycles (not a battery-domain
 * wipe) and are the standard place for a monotonic anti-rollback counter. */
#define TAMP_BASE          0x40007000UL
#define TAMP_BKP0R         (*(volatile uint32_t *)(TAMP_BASE + 0x100UL))

static uint8_t fw_hash[32];
static const uint8_t expected_magic[8] = "STSECAPP";

/**
 * @brief   Enables backup-domain write access so TAMP_BKP0R can be
 *          updated. Backup registers can always be *read* without this,
 *          but writing requires DBP (PWR->CR1) - WB55 has no PWR clock
 *          gating bit (PWR is always clocked), so only DBP + the RTC/TAMP
 *          APB clock need to be set here.
 */
static void secure_boot_enable_backup_domain(void)
{
    PWR->CR1 |= PWR_CR1_DBP;
    RCC->APB1ENR1 |= RCC_APB1ENR1_RTCAPBEN;
    (void)RCC->APB1ENR1;
}

/**
 * @brief  Flash is memory-mapped and readable directly by the CPU; no
 *         unlock/lock sequence is needed for reads (only for
 *         program/erase), so this simply memcpy()s out of flash.
 */
static void flash_read(uint32_t addr, uint8_t *dst, uint32_t len)
{
    memcpy(dst, (const void *)addr, len);
}

/**
 * @brief  Reads the current readout-protection level directly from the
 *         option-byte register (replaces HAL_FLASHEx_OBGetRDP()).
 */
static uint32_t flash_get_rdp_level(void)
{
    return (FLASH->OPTR & FLASH_OPTR_RDP) >> FLASH_OPTR_RDP_Pos;
}

int SecureBoot_VerifyFirmware(void)
{
    uint8_t header[64];
    uint32_t fw_size;

    // 1. Read & validate header (flash is memory-mapped - no unlock needed to read)
    flash_read(FW_START_ADDR, header, 64);

    // Check magic
    if (memcmp(header, expected_magic, 8) != 0) {
        return -2;  // Invalid magic
    }

    fw_size = *(uint32_t*)(header + 8);
    if ((fw_size > FW_MAX_SIZE) || (fw_size < (1024U + SIGNATURE_SIZE))) {
        return -3;  // Invalid size (must also leave room for the signature)
    }

    // 2. Compute hash of FW data (excluding the trailing signature).
    uint32_t payload_len = fw_size - SIGNATURE_SIZE;
    uint32_t signature_offset = payload_len;
    CRYPTO_SHA256((uint8_t*)FW_START_ADDR, payload_len, fw_hash);

    // 3. Read signature from flash (64 bytes: 32-byte R || 32-byte S)
    uint8_t signature[ECDSA_SIGNATURE_LEN];
    flash_read(FW_START_ADDR + signature_offset, signature, ECDSA_SIGNATURE_LEN);

    // 4. Read public key from provisioned flash (64 bytes: X || Y)
    uint8_t pubkey[ECDSA_PUBKEY_LEN];
    if (flash_get_rdp_level() == OB_RDP_LEVEL_0) {
        /* RDP level 0 is the shipped-default on an unprovisioned board,
         * not a "we're definitely in a debug session" signal - so
         * treating it as automatic permission to skip signing is a
         * live full auth bypass on any board that hasn't had RDP
         * raised yet. Fail closed unless SECUREBOOT_ALLOW_UNSIGNED_RDP0
         * is explicitly defined by the build, so skipping verification
         * is a deliberate, visible opt-in rather than the default. */
#ifdef SECUREBOOT_ALLOW_UNSIGNED_RDP0
        memset(pubkey, 0, sizeof(pubkey));
#else
        return -6;  // RDP0: refusing to boot an unsigned image
#endif
    } else {
        flash_read(PUBKEY_ADDR, pubkey, ECDSA_PUBKEY_LEN);
    }

    // 5. Verify ECDSA signature using PKA accelerator
    if (!ECDSA_VerifyP256(pubkey, fw_hash, signature)) {
        return -4;  // Signature invalid
    }

    // 6. Anti-rollback check (version in header) against a real monotonic
    // counter held in a TAMP backup register.
    uint32_t fw_version = *(uint32_t*)(header + 12);
    uint32_t min_version = TAMP_BKP0R;

    if (fw_version < min_version) {
        return -5;  // Rollback detected
    }

    if (fw_version > min_version) {
        secure_boot_enable_backup_domain();
        TAMP_BKP0R = fw_version;
    }

    return 0;  // Verified OK
}

void SecureBoot_JumpToBootloader(void)
{
    // System bootloader jump (UART/USB)
    void (*SysBootJump)(void) = (void*)0x1FFF7800;

    // Disable all peripherals (register-level equivalent of HAL_RCC_DeInit():
    // fall back to MSI-only, clear prescalers/peripheral clock enables).
    RCC->CR |= RCC_CR_MSION;
    while ((RCC->CR & RCC_CR_MSIRDY) == 0U) { }
    RCC->CFGR &= ~RCC_CFGR_SW;              /* switch SYSCLK back to MSI */
    while ((RCC->CFGR & RCC_CFGR_SWS) != 0U) { }
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_HSION);
    RCC->CFGR = 0U;
    RCC->AHB1ENR = 0U;
    RCC->AHB2ENR = 0U;
    RCC->AHB3ENR = 0U;
    RCC->APB1ENR1 = 0U;
    RCC->APB1ENR2 = 0U;
    RCC->APB2ENR = 0U;
    RCC->CIER = 0U;

    SysTick->CTRL = 0;
    __disable_irq();

    // Jump
    SysBootJump();
}
