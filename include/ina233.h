#ifndef INA233_H
#define INA233_H

#include <stdbool.h>
#include <stdint.h>

#define INA233_REG_BUS_VOLTAGE 0x05u
#define INA233_REG_CURRENT 0x07u
#define INA233_REG_POWER 0x08u
#define INA233_REG_ENERGY 0x09u
#define INA233_REG_CHARGE 0x0Au

typedef struct {
	uint16_t bus_voltage_raw;
	uint16_t current_raw;
	uint16_t power_raw;
	uint16_t energy_raw;
	uint16_t charge_raw;
} ina233_raw_sample_t;

bool ina233_read_register(uint8_t command, uint16_t *value);
bool ina233_read_sample(ina233_raw_sample_t *sample);

#endif
