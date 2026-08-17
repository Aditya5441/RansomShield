TARGET   := Ransomware_detection_core
BUILD    := Debug
LDSCRIPT := STM32WB55RGVX_FLASH.ld

MCU      := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS   := $(MCU) -std=c11 -Wall -g3 -O0 \
            -DSTM32WB55xx -DDEBUG \
            -ffunction-sections -fdata-sections
ASFLAGS  := $(MCU) -g3
LDFLAGS  := $(MCU) -T$(LDSCRIPT) --specs=nosys.specs --specs=nano.specs \
            -Wl,-Map=$(BUILD)/$(TARGET).map,--gc-sections -static \
            -Wl,--start-group -lc -lm -Wl,--end-group

INCLUDES := \
	-ICore/Inc \
	-IDrivers/BareMetal \
	-IDrivers/CMSIS/Device/ST/STM32WBxx/Include \
	-IDrivers/CMSIS/Include \
	-IDrivers/CMSIS/RTOS2/Include \
	-IMiddlewares/Third_Party/FreeRTOS/Source/include \
	-IMiddlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 \
	-IMiddlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
	-IApplication/Sensors \
	-IApplication/DSP \
	-IApplication/Security \
	-IApplication/Communication \
	-IApplication/AI \
	-IX-CUBE-AI/App \
	-IX-CUBE-AI

C_SRCS := \
	Core/Src/main.c \
	Core/Src/app_freertos.c \
	Core/Src/stm32wbxx_it.c \
	Core/Src/system_stm32wbxx.c \
	Drivers/BareMetal/bm_adc.c \
	Drivers/BareMetal/bm_aes.c \
	Drivers/BareMetal/bm_board.c \
	Drivers/BareMetal/bm_crc_pka.c \
	Drivers/BareMetal/bm_dma.c \
	Drivers/BareMetal/bm_gpio.c \
	Drivers/BareMetal/bm_i2c.c \
	Drivers/BareMetal/bm_init.c \
	Drivers/BareMetal/bm_rcc.c \
	Drivers/BareMetal/bm_rng.c \
	Drivers/BareMetal/bm_spi.c \
	Drivers/BareMetal/bm_tim.c \
	Drivers/BareMetal/bm_uart.c \
	Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c \
	Middlewares/Third_Party/FreeRTOS/Source/croutine.c \
	Middlewares/Third_Party/FreeRTOS/Source/event_groups.c \
	Middlewares/Third_Party/FreeRTOS/Source/list.c \
	Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
	Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
	Middlewares/Third_Party/FreeRTOS/Source/queue.c \
	Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c \
	Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
	Middlewares/Third_Party/FreeRTOS/Source/timers.c \
	Application/Sensors/sensors.c \
	Application/Sensors/ina219.c \
	Application/Sensors/mpu6050.c \
	Application/Sensors/lis3dh.c \
	Application/Sensors/ntc.c \
	Application/Sensors/voltage.c \
	Application/Sensors/nrf24.c \
	Application/Sensors/si4432.c \
	Application/Sensors/w5500.c \
	Application/Sensors/Core/syscalls.c \
	Application/Sensors/Core/sysmem.c \
	Application/DSP/dsp.c \
	Application/DSP/wvd.c \
	Application/DSP/cs.c \
	Application/Security/comm_encryption.c \
	Application/Security/chaos.c \
	Application/Security/feature_protection.c \
	Application/Communication/esp32.c \
	Application/AI/ai.c \
	Application/AI/ai_inputs.c \
	X-CUBE-AI/App/app_x-cube-ai.c

ASM_SRCS := \
	Startup/startup_stm32wb55rgvx.s

OBJS := $(addprefix $(BUILD)/,$(notdir $(C_SRCS:.c=.o)))
OBJS += $(addprefix $(BUILD)/,$(notdir $(ASM_SRCS:.s=.o)))

VPATH := $(sort $(dir $(C_SRCS) $(ASM_SRCS)))

.PHONY: all clean size train keras-weights

all: $(BUILD)/$(TARGET).elf size

train:
	python3 tools/train_ransomware_model.py

keras-weights:
	python3 tools/generate_keras_weights_h.py

$(BUILD)/$(TARGET).elf: $(OBJS) $(LDSCRIPT) Makefile
	@mkdir -p $(BUILD)
	arm-none-eabi-gcc $(OBJS) $(LDFLAGS) -o $@

$(BUILD)/%.o: %.c Makefile
	@mkdir -p $(BUILD)
	arm-none-eabi-gcc $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/%.o: %.s Makefile
	@mkdir -p $(BUILD)
	arm-none-eabi-gcc $(ASFLAGS) -c $< -o $@

size: $(BUILD)/$(TARGET).elf
	arm-none-eabi-size $(BUILD)/$(TARGET).elf

clean:
	rm -rf $(BUILD)
