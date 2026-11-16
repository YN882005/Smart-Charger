# ATmega328P Smart Charger Firmware Skeleton

This repository contains an AVR-GCC project for an ATmega328P and INA233.
It is a **monitor-only, non-charging skeleton**: the charge PWM output is held
high (inactive for a direct P-channel MOSFET gate) and the discharge MOSFET
output is held low. There is intentionally no path that enables either power
stage.

## Hardware assumptions to verify

- The firmware assumes a 16 MHz ATmega328P clock; override `F_CPU` in the
  Makefile if the board clock differs.
- INA233 I2C address defaults to `0x40`. Confirm the address straps and bus
  voltage against the actual board.
- The charge output is PB1/OC1A and the discharge output is PD7. The default
  inactive levels assume PB1 directly represents an active-low P-MOSFET gate
  command and PD7 an active-high N-MOSFET gate command. Verify the complete
  gate-driver schematic and add external pull resistors so both stages remain
  off while the MCU is reset, unpowered, or its pins are high-impedance.
- Connect the INA233 to the ATmega328P TWI pins (PC4/SDA and PC5/SCL) with
  correctly sized pull-ups. This firmware reads raw register words only; it
  does not configure INA233 calibration or convert measurements to engineering
  units.

These are firmware pin assumptions, not a validated power-stage design.
An LC-filtered PWM gate drive, back-to-back MOSFET arrangement, battery
protection, and resistor-discharge path must be reviewed and tested separately.
Do not connect a battery or energize the power stage based only on this
skeleton.

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

## Firmware contents

- `include/config.h`: board and safe-default configuration.
- `include/board.h`, `src/board.c`: output initialization and safe-off controls.
- `include/twi.h`, `src/twi.c`: bounded-timeout AVR TWI master and word reads.
- `include/ina233.h`, `src/ina233.c`: INA233 raw register reads.
- `src/main.c`: safe initialization and periodic measurement polling.

`g_ina233_sample_valid` and `g_ina233_sample` are debugger-visible. The sample
contains raw register words, not scaled volts, amps, watts, watt-hours, or
amp-hours. A failed I2C transaction marks the sample invalid; the power outputs
remain in their safe states.

## Before implementing charging

Specify and independently validate the cell count and exact cell
manufacturer-approved charge limits for each supported chemistry (Li-ion/LiPo
and LiFePO4), shunt resistance and INA233 calibration, charge-current and
temperature limits, sensor placement, cell balancing/monitoring, fault
handling, and hardware over-current/over-voltage protections. A pack-level
voltage reading alone cannot detect an individual series-cell over-voltage.
Do not enable charging until the full schematic, control-loop stability,
fault response, and battery-specific limits have been reviewed and tested.
