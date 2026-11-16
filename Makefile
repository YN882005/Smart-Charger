MCU       ?= atmega328p
F_CPU     ?= 16000000
AVR_CC       ?= avr-gcc
AVR_OBJCOPY  ?= avr-objcopy
AVR_SIZE     ?= avr-size
BUILD_DIR := build

SOURCES := \
	src/main.c \
	src/board.c \
	src/twi.c \
	src/ina233.c

CFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -std=gnu11 -Os \
	-Wall -Wextra -Werror -Iinclude -ffunction-sections -fdata-sections
LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections

.PHONY: all clean size

all: $(BUILD_DIR)/firmware.hex

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/firmware.elf: $(SOURCES) include/config.h include/board.h include/twi.h include/ina233.h | $(BUILD_DIR)
	$(AVR_CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $@
	$(AVR_SIZE) $@

$(BUILD_DIR)/firmware.hex: $(BUILD_DIR)/firmware.elf
	$(AVR_OBJCOPY) -O ihex -R .eeprom $< $@

size: $(BUILD_DIR)/firmware.elf
	$(AVR_SIZE) $<

clean:
	rm -rf $(BUILD_DIR)
