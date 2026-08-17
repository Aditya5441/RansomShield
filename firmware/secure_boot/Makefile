##
## Baremetal build for the STM32WB55 secure-boot stage.
## No STM32 HAL, no STM32CubeIDE - CMSIS register headers + direct register
## access only. Requires the arm-none-eabi GCC toolchain.
##

TARGET      = secure_boot
BUILD_DIR   = build

PREFIX      = arm-none-eabi-
CC          = $(PREFIX)gcc
OBJCOPY     = $(PREFIX)objcopy
OBJDUMP     = $(PREFIX)objdump
SIZE        = $(PREFIX)size

# --- Sources -----------------------------------------------------------
C_SOURCES = \
  Core/Src/main.c \
  Core/Src/bootloader.c \
  Core/Src/secure_boot.c \
  Core/Src/secure_vault.c \
  Core/Src/crypto.c \
  Core/Src/ecdsa_verify.c \
  Core/Src/stm32wbxx_it.c \
  Core/Src/system_stm32wbxx.c \
  Core/Src/syscalls.c \
  Core/Src/sysmem.c

ASM_SOURCES = Core/Startup/startup_stm32wb55rgvx.s

# --- Includes ------------------------------------------------------------
C_INCLUDES = \
  -ICore/Inc \
  -IDrivers/CMSIS/Device/ST/STM32WBxx/Include \
  -IDrivers/CMSIS/Include

# --- MCU flags -----------------------------------------------------------
MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

C_DEFS = -DSTM32WB55xx

CFLAGS  = $(MCU) $(C_DEFS) $(C_INCLUDES) -Wall -Wextra -Os -g3 -ffunction-sections -fdata-sections -std=c11
ASFLAGS = $(MCU) -g3

LDSCRIPT = STM32WB55RGVX_FLASH.ld
LDFLAGS  = $(MCU) -specs=nano.specs -specs=nosys.specs -T$(LDSCRIPT) \
           -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -lc -lm -lnosys

OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(CC) -x assembler-with-cpp -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -O binary -S $< $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
