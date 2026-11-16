#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>

/* ATmega328P-PU external crystal frequency: 14.7456 MHz */
#ifndef F_CPU
#define F_CPU 14745600UL
#endif

/* -------------------------------------------------------------------------
 * I2C / TWI Configuration
 * ------------------------------------------------------------------------- */
#define TWI_CLOCK_HZ           100000UL
#define OLED_I2C_ADDRESS       0x3Cu

/* INA233 7-bit I2C addresses (from hardware schematic):
 * CH1: U6 (A1=GND, A0=GND) -> 0x40
 * CH2: U7 (A1=VCC, A0=VCC) -> 0x45
 * CH3: U8 (A1=GND, A0=VCC) -> 0x41 (Expansion)
 * CH4: U9 (A1=VCC, A0=GND) -> 0x44 (Expansion)
 */
#define INA233_CH1_I2C_ADDRESS 0x40u
#define INA233_CH2_I2C_ADDRESS 0x45u
#define INA233_CH3_I2C_ADDRESS 0x41u
#define INA233_CH4_I2C_ADDRESS 0x44u

/* Shunt resistance: 3 x 100 mOhm in parallel = 33.333 mOhm */
#define INA233_R_SHUNT_OHMS    0.033333333f
#define INA233_I_MAX_AMPS      2.0f

/* Keep all power-stage control disabled until independently safety-validated. */
#define POWER_STAGE_CONTROL_ENABLED 0u
#if POWER_STAGE_CONTROL_ENABLED
#error "Power-stage control must remain disabled pending independent safety validation."
#endif

/* Shared INA233 Alert line: PD5 (active-low, 5.1 kOhm pull-up) */
#define INA233_ALERT_DDR       DDRD
#define INA233_ALERT_PORT      PORTD
#define INA233_ALERT_PIN       PIND
#define INA233_ALERT_BIT       PD5

/* -------------------------------------------------------------------------
 * Rotary Encoder HW-040 Configuration
 * CLK = PC1, DT = PC2, Switch = PC3 (active-low, 100 nF hardware debounce C17)
 * ------------------------------------------------------------------------- */
#define ENCODER_DDR            DDRC
#define ENCODER_PORT           PORTC
#define ENCODER_PIN            PINC
#define ENCODER_CLK_BIT        PC1
#define ENCODER_DT_BIT         PC2
#define ENCODER_SWITCH_BIT     PC3

/* -------------------------------------------------------------------------
 * Buzzer Configuration
 * Pin: PD4 (T0) via 47 Ohm series resistor (R46)
 * Target tone: 2.3 kHz (2304 Hz exact with 14.7456 MHz /64 prescaler)
 * Menu transition tone duration: 400 ms
 * ------------------------------------------------------------------------- */
#define BUZZER_DDR             DDRD
#define BUZZER_PORT            PORTD
#define BUZZER_PIN             PIND
#define BUZZER_BIT             PD4
#define BUZZER_FREQUENCY_HZ    2304UL
#define BUZZER_BEEP_MS         400u

/* -------------------------------------------------------------------------
 * Power Stages Pin Mapping
 * CH1 Charge PWM:     PB1 (OC1A) - Active-LOW P-MOSFET (AO3415)
 * CH1 Discharge Gate: PB0        - Active-HIGH N-MOSFET (SI2300DS)
 * CH2 Charge PWM:     PB2 (OC1B) - Active-LOW P-MOSFET (AO3415)
 * CH2 Discharge Gate: PB5        - Active-HIGH N-MOSFET (SI2300DS)
 *
 * Expansion pins:
 * CH3 Charge PWM:     PB3 (OC2A) - Active-LOW P-MOSFET
 * CH3 Discharge Gate: PB4        - Active-HIGH N-MOSFET
 * CH4 Charge PWM:     PD3 (OC2B) - Active-LOW P-MOSFET
 * CH4 Discharge Gate: PC0        - Active-HIGH N-MOSFET
 * ------------------------------------------------------------------------- */
#define CHARGE1_BIT            PB1
#define CHARGE2_BIT            PB2
#define CHARGE3_BIT            PB3
#define CHARGE4_BIT            PD3

#define DISCHARGE1_BIT         PB0
#define DISCHARGE2_BIT         PB5
#define DISCHARGE3_BIT         PB4
#define DISCHARGE4_BIT         PC0

/* -------------------------------------------------------------------------
 * Battery Chemistry & Threshold Limits (in millivolts and milliamperes)
 * ------------------------------------------------------------------------- */
#define BATT_PRESENCE_THRESHOLD_MV 1000u /* < 1.0 V treated as disconnected */

/* Reference limits only; no charge/discharge control applies these values. */
#define CH1_FULL_CHARGE_MV     4200u
#define CH1_SAFE_CUTOFF_MV     3000u

#define CH2_FULL_CHARGE_MV     3650u
#define CH2_SAFE_CUTOFF_MV     2500u

/* GUI preview range only; it does not set a hardware current limit. */
#define CHG_CURRENT_MIN_MA     100u
#define CHG_CURRENT_MAX_MA     1200u
#define CHG_CURRENT_STEP_MA    100u
#define CHG_CURRENT_DEFAULT_MA 500u
#define CHG_CUTOFF_CURRENT_MA  50u

#endif /* CONFIG_H */
