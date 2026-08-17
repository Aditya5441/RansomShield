#include "secure_boot.h"
#include "crypto.h"
#include "ecdsa_verify.h"
#include "bootloader.h"
#include "main.h"
#include "stm32wbxx_hal.h"
#include <string.h>

/* This stage (Bootloader) verifies the NEXT stage in the chain: the
 * application image. APP_ADDR/FLASH_END_ADDR come from bootloader.h and
 * must stay in sync with Ransomware_detection_core's own linker script
 * (ORIGIN = 0x08010000, now 0x08010100 for the code region - see
 * bootloader.h APP_CODE_ADDR). */
#define FW_START_ADDR      APP_ADDR
#define FW_MAX_SIZE        (FLASH_END_ADDR - APP_ADDR + 1U)
#define SIGNATURE_SIZE     ECDSA_SIGNATURE_LEN  // Real P-256 sig = 64 bytes
#define PUBKEY_ADDR        0x08800000UL  /* Same provisioned-key flash
 * sector as Secure Boot uses to verify THIS bootloader - both stages
 * currently trust the same key. Using one key for both stages is
 * simpler to provision but means a single compromised private key
 * forges both bootloader and app images; a separate key per stage is
 * the more defensible design if this becomes a real product. */

/* Separate backup register from Secure Boot's BKP0R (used for the
 * bootloader image's own version), so the bootloader and app rollback
 * counters can't collide/overwrite each other. */
#define TAMP_BASE          0x40007000UL
#define TAMP_BKP1R         (*(volatile uint32_t *)(TAMP_BASE + 0x104UL))

static uint8_t fw_hash[32];
static const uint8_t expected_magic[8] = "STSECAPP";

static void secure_boot_enable_backup_domain(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
}

int SecureBoot_VerifyFirmware(void)
{
    uint8_t header[64];
    uint32_t fw_size;

    HAL_FLASH_Unlock();
    if (HAL_FLASHEx_Read(FW_START_ADDR, header, 64) != HAL_OK) {
        HAL_FLASH_Lock();
        return -1;
    }
    HAL_FLASH_Lock();

    if (memcmp(header, expected_magic, 8) != 0) {
        return -2;  // Invalid magic
    }

    fw_size = *(uint32_t*)(header + 8);
    if ((fw_size > FW_MAX_SIZE) || (fw_size < (1024U + SIGNATURE_SIZE))) {
        return -3;  // Invalid size
    }

    uint32_t payload_len = fw_size - SIGNATURE_SIZE;
    uint32_t signature_offset = payload_len;
    CRYPTO_SHA256((uint8_t*)FW_START_ADDR, payload_len, fw_hash);

    uint8_t signature[ECDSA_SIGNATURE_LEN];
    HAL_FLASH_Unlock();
    HAL_FLASHEx_Read(FW_START_ADDR + signature_offset, signature, ECDSA_SIGNATURE_LEN);
    HAL_FLASH_Lock();

    uint8_t pubkey[ECDSA_PUBKEY_LEN];
    if (HAL_FLASHEx_OBGetRDP() == OB_RDP_LEVEL_0) {
        /* See Secure_boot/Core/Src/secure_boot.c for why RDP0 fails
         * closed by default instead of silently allowing unsigned
         * images - RDP0 is the shipped-default, not proof of an
         * intentional debug session. */
#ifdef SECUREBOOT_ALLOW_UNSIGNED_RDP0
        memset(pubkey, 0, sizeof(pubkey));
#else
        return -6;
#endif
    } else {
        memcpy(pubkey, (void*)PUBKEY_ADDR, ECDSA_PUBKEY_LEN);
    }

    if (!ECDSA_VerifyP256(pubkey, fw_hash, signature)) {
        return -4;  // Signature invalid
    }

    uint32_t fw_version = *(uint32_t*)(header + 12);
    uint32_t min_version = TAMP_BKP1R;

    if (fw_version < min_version) {
        return -5;  // Rollback detected
    }

    if (fw_version > min_version) {
        secure_boot_enable_backup_domain();
        TAMP_BKP1R = fw_version;
    }

    return 0;
}
