#ifndef CONFIG_H
#define CONFIG_H

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define INA233_I2C_ADDRESS 0x40u
#define TWI_CLOCK_HZ 100000UL

/* This project is monitor-only until a validated control and safety design exists. */
#define CHARGER_ENABLE 0

#define CHARGE_PWM_BIT PB1
#define DISCHARGE_GATE_BIT PD7

#if CHARGER_ENABLE
#error "Charging control is not implemented; keep CHARGER_ENABLE set to 0."
#endif

#endif
