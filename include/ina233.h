#ifndef INA233_H
#define INA233_H

#include <stdbool.h>
#include <stdint.h>
#include "infinityPV_INA233.h"

/* PMBus / INA233 Register Command aliases */
#define INA233_REG_BUS_VOLTAGE   READ_VIN
#define INA233_REG_SHUNT_VOLTAGE MFR_READ_VSHUNT
#define INA233_REG_CURRENT       READ_IIN
#define INA233_REG_POWER         READ_PIN
#define INA233_REG_ENERGY        READ_EIN
#define INA233_REG_CHARGE        READ_EIN

typedef struct {
	uint16_t bus_voltage_raw;    /* READ_VIN (0x88) */
	uint16_t shunt_voltage_raw;  /* MFR_READ_VSHUNT (0xD1) */
	uint16_t current_raw;        /* READ_IIN (0x89) */
	uint16_t power_raw;          /* READ_PIN (0x97) */
	uint16_t energy_raw;         /* READ_EIN accumulator (0x86) */
	uint16_t charge_raw;         /* READ_EIN sample count lower word */
	float bus_voltage_v;         /* getBusVoltage_V() */
	float shunt_voltage_mv;      /* getShuntVoltage_mV() */
	float current_ma;            /* getCurrent_mA() */
	float power_mw;              /* getPower_mW() */
} ina233_raw_sample_t;

void ina233_device_init(uint8_t address, float r_shunt, float i_max);
bool ina233_read_register(uint8_t command, uint16_t *value);
bool ina233_read_sample(ina233_raw_sample_t *sample);
ina233_t *ina233_get_default_device(void);

#endif
