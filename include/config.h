#ifndef CONFIG_H
#define CONFIG_H

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define INA233_I2C_ADDRESS 0x40u
#define OLED_I2C_ADDRESS 0x3Cu
#define TWI_CLOCK_HZ 100000UL

/* Keep monitor-only until a validated control and safety design exists. */
#define CHARGER_ENABLE 0

#define CHARGE1_BIT PB1
#define CHARGE2_BIT PB2
#define CHARGE3_BIT PB3
#define CHARGE4_BIT PD3

#define DISCHARGE1_BIT PB0
#define DISCHARGE2_BIT PB5
#define DISCHARGE3_BIT PB4
#define DISCHARGE4_BIT PC0

#define ENCODER_CLK_BIT PC1
#define ENCODER_DT_BIT PC2
#define ENCODER_SWITCH_BIT PD5
#define BUZZER_BIT PD4
#define BUZZER_FREQUENCY_HZ 2300UL

#if CHARGER_ENABLE
#error "Charging control is not implemented; keep CHARGER_ENABLE set to 0."
#endif

#endif
