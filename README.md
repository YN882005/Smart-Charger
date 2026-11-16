# ATmega328P Smart Charger Firmware

This repository contains an AVR-GCC project for an ATmega328P-PU, two INA233
sensors, an I2C OLED, an HW-040 rotary encoder, and a 2.304 kHz buzzer. It is
**monitor-only**: charge outputs are held high, discharge outputs are held
low, and the firmware has no path that enables a power stage.

## Schematic pin mapping

- The default clock is the specified 14.7456 MHz external crystal; override
  `F_CPU` only if the board clock differs.
- CH1 uses INA233 address `0x40` and CH2 uses `0x45`. Both use the specified
  33.333 mOhm shunt. CH3 and CH4 are not sampled.
- Connect both sensors and the OLED to PC4/SDA and PC5/SCL. The OLED address
  is `0x3C`; confirm its SSD1306 compatibility and the specified bus pull-ups.
- The encoder uses CLK=PC1, DT=PC2, and switch=PC3. The shared active-low
  INA233 ALERT line is on PD5. Encoder inputs use internal pull-ups.
- The buzzer is on PD4 and generates 2.304 kHz tones for 400 ms on menu
  transitions.

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
- `include/Charging.h`, `src/Charging.c`: charging stage output initialization and safe-off controls.
- `include/Discharging.h`, `src/Discharging.c`: discharging stage output initialization and safe-off controls.
- `include/encoder.h`, `src/encoder.c`: debounced rotary encoder, switch, and double-press detection.
- `include/buzzer.h`, `src/buzzer.c`: non-blocking Timer2 buzzer tone.
- `include/OLED_Screen.h`, `src/OLED_Screen.c`: OLED screen driver, telemetry, and menu UI.
- `include/twi.h`, `src/twi.c`: bounded-timeout AVR TWI master, word/byte/block transfers, and PMBus primitives.
- `include/ina233.h`, `src/ina233.c`: INA233 PMBus driver, calibration, register sampling, and unit conversion.
- `src/main.c`: safe initialization, polling, and UI service.

The main screen shows both channel voltages and currents, sensor/presence
status, and the shared ALERT input. State of charge is marked unavailable
until validated cell-specific lookup tables are supplied. Rotate to select a
channel, then use the mode and current-limit menus. These menus are preview
only: confirming a setting displays `CONTROL LOCKED`; no charge/discharge
output can be enabled. The cycle timer and charge/discharge animation are
inactive because no power cycle can be started. A double press returns to the
main screen.

`g_ina233_sample_valid` and `g_ina233_samples` expose per-channel samples for
debugging, along with `g_oled_ready`. A failed transaction invalidates only
the affected channel's sample. The power outputs remain off.

## Before implementing charging

Specify and independently validate the cell count and exact cell
manufacturer-approved charge limits for each supported chemistry (Li-ion/LiPo
and LiFePO4), shunt resistance and INA233 calibration, charge-current and
temperature limits, sensor placement, cell balancing/monitoring, fault
handling, and hardware over-current/over-voltage protections. A pack-level
voltage reading alone cannot detect an individual series-cell over-voltage.
The charge/discharge settings in the GUI are intentionally non-operational.
`POWER_STAGE_CONTROL_ENABLED` is compile-time guarded off, and the firmware
contains no path to energize a power stage. The supplied specification does
not include validated state-of-charge tables for the exact cells, so the GUI
reports SoC as unavailable rather than displaying an unsupported estimate.

Do not enable charging until the full schematic, control-loop stability,
fault response, and battery-specific limits have been independently reviewed
and tested.
