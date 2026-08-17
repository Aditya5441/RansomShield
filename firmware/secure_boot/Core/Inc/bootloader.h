#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

/**
 * Runs SecureBoot_VerifyFirmware() against the next-stage Bootloader image
 * and, on success, jumps into it. On failure it falls back to the system
 * bootloader (SecureBoot_JumpToBootloader()) for recovery/reflash.
 *
 * NOTE: this stub was NOT part of the uploaded project - main.c calls
 * Bootloader_Run() but bootloader.h/.c weren't included in the zip (the
 * comments in secure_boot.c make clear the verified image is a separate
 * "Bootloader" project at flash offset 0x08008000). This minimal
 * implementation is added purely so this stage builds and links standalone;
 * replace it with the real next-stage entry logic for this project.
 */
void Bootloader_Run(void);

#endif /* INC_BOOTLOADER_H_ */
