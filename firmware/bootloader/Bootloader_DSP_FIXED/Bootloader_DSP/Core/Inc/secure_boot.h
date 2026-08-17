/* secure_boot.h
 * Bootloader-stage counterpart to Secure_boot's own SecureBoot_VerifyFirmware():
 * Secure Boot verifies THIS bootloader image before jumping to it; this
 * verifies the APPLICATION image before this bootloader jumps to it.
 * Same magic/header/ECDSA/anti-rollback scheme, different flash slot.
 */
#ifndef INC_SECURE_BOOT_H_
#define INC_SECURE_BOOT_H_

int SecureBoot_VerifyFirmware(void);

#endif /* INC_SECURE_BOOT_H_ */
