# Makefile for GD32F130C8 (Hoverboard Gen2)
# Target executable name
TARGET = hover

# Toolchain
CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc -x assembler-with-cpp
CP = arm-none-eabi-objcopy
SZ = arm-none-eabi-size

# Build directory
BUILD_DIR = build

# Source files
SOURCES = \
Src/main.c \
Src/bldc.c \
Src/comms.c \
Src/commsBluetooth.c \
Src/commsMasterSlave.c \
Src/commsSteering.c \
Src/it.c \
Src/led.c \
Src/setup.c \
RTE/Device/GD32F130C8/gd32f1x0_adc.c \
RTE/Device/GD32F130C8/gd32f1x0_dbg.c \
RTE/Device/GD32F130C8/gd32f1x0_dma.c \
RTE/Device/GD32F130C8/gd32f1x0_fwdgt.c \
RTE/Device/GD32F130C8/gd32f1x0_gpio.c \
RTE/Device/GD32F130C8/gd32f1x0_i2c.c \
RTE/Device/GD32F130C8/gd32f1x0_misc.c \
RTE/Device/GD32F130C8/gd32f1x0_opa.c \
RTE/Device/GD32F130C8/gd32f1x0_pmu.c \
RTE/Device/GD32F130C8/gd32f1x0_rcu.c \
RTE/Device/GD32F130C8/gd32f1x0_syscfg.c \
RTE/Device/GD32F130C8/gd32f1x0_timer.c \
RTE/Device/GD32F130C8/gd32f1x0_usart.c \
RTE/Device/GD32F130C8/gd32f1x0_wwdgt.c \
RTE/Device/GD32F130C8/system_gd32f1x0.c

# Startup file
ASSEMBLY_SOURCES = \
Include/startup_gd32f1x0.s

# Includes
INCLUDES = \
-IInc \
-IInclude \
-IRTE/Device/GD32F130C8

# Compiler flags
MCU = -mcpu=cortex-m3 -mthumb
DEFINES = -DGD32F1x0 -DGD32F130_150 -DUSE_STDPERIPH_DRIVER -DARM_MATH_CM3
CFLAGS = $(MCU) $(DEFINES) $(INCLUDES) -O2 -Wall -fdata-sections -ffunction-sections
ASFLAGS = $(MCU) $(AS_DEFS) $(INCLUDES) -O2 -Wall -fdata-sections -ffunction-sections

# Linker flags
LDSCRIPT = gd32f130c8.ld
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--gc-sections

# Object files
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASSEMBLY_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASSEMBLY_SOURCES)))

# Build targets
all: $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(CP) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(CP) -O binary -S $< $@

$(BUILD_DIR):
	mkdir $@

clean:
	rm -rf $(BUILD_DIR)

# Custom target for MASTER/SLAVE builds
master:
	$(MAKE) DEFINES="$(DEFINES) -DMASTER" TARGET=hover_master all

slave:
	$(MAKE) DEFINES="$(DEFINES) -DSLAVE" TARGET=hover_slave all

.PHONY: all clean master slave
