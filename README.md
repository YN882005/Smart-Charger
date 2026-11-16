# ATmega328P Smart Charger Firmware

This repository contains an AVR-GCC project for an ATmega328P, INA233, I2C
OLED, HW-040 rotary encoder, and 2.3 kHz buzzer. It is **monitor-only and
non-charging**. All four charge outputs are held high, all four discharge
outputs are held low, and there is intentionally no path that enables a power
stage.

## Schematic pin mapping

- The firmware assumes a 16 MHz ATmega328P clock; override `F_CPU` in the
  Makefile if the board clock differs.
- The schematic maps CHG1-4 to PB1, PB2, PB3, and PD3; DSG1-4 map to PB0,
  PB5, PB4, and PC0, respectively. Firmware initializes each output to its
  inactive level before enabling the output driver.
- Connect the INA233 and OLED to the shared I2C bus: PC4/SDA and PC5/SCL.
  INA233 defaults to address `0x40`; the OLED defaults to `0x3C`. Confirm the
  address straps, display controller compatibility (SSD1306), and bus
  pull-ups.
- The HW-040 signals in the schematic are CLK=PC1, DT=PC2, and switch/Alert=PD5.
  The firmware enables internal pull-ups and uses pin-change interrupts for
  rotation.
- The buzzer control net is PD4. The firmware generates a 2.3 kHz tone using
  Timer2; the schematic's transistor stage must drive the 5 V buzzer.

Verify output polarity and the complete gate-driver schematic before
programming. Add external pull resistors so every charge/discharge stage
remains off while the MCU is reset, unpowered, or its pins are high-impedance.
The firmware's inactive levels cannot substitute for hardware interlocks. This
schematic-based mapping is not a validation of the power-stage design. Do not
connect a battery or energize the power stage based only on this code.

## Build

Install `avr-gcc`, `avr-libc`, GNU Make, and AVR binutils, then run:

```sh
make
```

The generated files are `build/firmware.elf` and `build/firmware.hex`.
The MCU and clock can be overridden, for example:

```sh
make MCU=atmega328p F_CPU=8000000
```

For a USBasp-compatible HW-437 programmer, a typical flash-only command is:

```sh
avrdude -c usbasp -p m328p -U flash:w:build/firmware.hex:i
```

Do not write fuse settings unless they have been checked against the board's
actual clock and reset configuration.

## Firmware contents

- `include/config.h`: board and safe-default configuration.
- `include/board.h`, `src/board.c`: output initialization and safe-off controls.
- `include/twi.h`, `src/twi.c`: bounded-timeout AVR TWI master and word reads.
- `include/ina233.h`, `src/ina233.c`: INA233 raw register reads.
- `include/ssd1306.h`, `src/ssd1306.c`: SSD1306 I2C display driver.
- `include/encoder.h`, `src/encoder.c`: debounced encoder/button input.
- `include/buzzer.h`, `src/buzzer.c`: non-blocking Timer2 buzzer tone.
- `include/ui.h`, `src/ui.c`: status and raw measurement screens.
- `src/main.c`: safe initialization, polling, and UI service.

The OLED shows a monitor-only status page and a second page of INA233 raw
register words. Rotate the encoder to switch pages; press it for an audio
acknowledgement. `g_ina233_sample_valid`, `g_ina233_sample`, and
`g_oled_ready` are debugger-visible. The sample contains raw register words,
not scaled volts, amps, watts, watt-hours, or amp-hours. A failed I2C
transaction marks the sample invalid; power outputs remain in their safe
states.

## Before implementing charging

Specify and independently validate the cell count and exact cell
manufacturer-approved charge limits for each supported chemistry (Li-ion/LiPo
and LiFePO4), shunt resistance and INA233 calibration, charge-current and
temperature limits, sensor placement, cell balancing/monitoring, fault
handling, and hardware over-current/over-voltage protections. A pack-level
voltage reading alone cannot detect an individual series-cell over-voltage.
Do not enable charging until the full schematic, control-loop stability,
fault response, and battery-specific limits have been reviewed and tested.
