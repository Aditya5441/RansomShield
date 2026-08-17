/* bootloader.h */
#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

#include <stdint.h>

/* APP_ADDR is where the 64-byte SecureBoot header (magic/size/version)
 * starts - this is what SecureBoot_VerifyFirmware() reads and hashes
 * from. The app's actual ARM vector table (SP + reset handler, which
 * BL_IsValidApp()/BL_JumpToApp() read) starts AFTER that header, at
 * APP_CODE_ADDR. Previously both checks pointed at APP_ADDR directly,
 * which meant they disagreed about what lives there - the plausibility
 * check would have been reading into the header's magic/size/version
 * bytes as if they were a stack pointer and reset vector.
 *
 * APP_HEADER_SIZE is rounded up from the 64 bytes SecureBoot actually
 * uses to a flash-write-friendly 256-byte block. The Ransomware_detection_
 * core app project's own linker script (STM32WB55RGVX_FLASH.ld) must have
 * its FLASH origin set to APP_CODE_ADDR (0x08010100), not APP_ADDR, and
 * the flashing/signing tool must write the 256-byte header at APP_ADDR
 * immediately before flashing the app image at APP_CODE_ADDR. */
#define APP_ADDR         0x08010000UL
#define APP_HEADER_SIZE  0x100UL                    /* 256 bytes */
#define APP_CODE_ADDR    (APP_ADDR + APP_HEADER_SIZE)
#define FLASH_END_ADDR   0x080FFFFFUL
#define SRAM_START_ADDR  0x20000000UL
#define SRAM_END_ADDR    0x2002FFFFUL

void Bootloader_Run(void);

#endif /* INC_BOOTLOADER_H_ */

