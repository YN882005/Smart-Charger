MCU       ?= atmega328p
F_CPU     ?= 16000000
AVR_CC       ?= avr-gcc
AVR_OBJCOPY  ?= avr-objcopy
AVR_SIZE     ?= avr-size
BUILD_DIR := build

SOURCES := \
	src/main.c \
	src/Charging.c \
	src/Discharging.c \
	src/twi.c \
	src/ina233.c \
	src/Encoder.c \
	src/Buzzer.c \
	src/OLED_Screen.c

HEADERS := \
	include/config.h \
	include/Charging.h \
	include/Discharging.h \
	include/twi.h \
	include/ina233.h \
	include/Encoder.h \
	include/Buzzer.h \
	include/OLED_Screen.h

CFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -std=gnu11 -Os \
	-Wall -Wextra -Werror -Iinclude -ffunction-sections -fdata-sections
LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections -lm

.PHONY: all clean size

all: $(BUILD_DIR)/firmware.hex

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/firmware.elf: $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(AVR_CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $@
	$(AVR_SIZE) $@

$(BUILD_DIR)/firmware.hex: $(BUILD_DIR)/firmware.elf
	$(AVR_OBJCOPY) -O ihex -R .eeprom $< $@

size: $(BUILD_DIR)/firmware.elf
	$(AVR_SIZE) $<

clean:
	rm -rf $(BUILD_DIR)
