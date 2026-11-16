#ifndef INA233_H
#define INA233_H

#include <stdbool.h>
#include <stdint.h>

#define INA233_REG_ENERGY        0x86u
#define INA233_REG_BUS_VOLTAGE   0x88u
#define INA233_REG_CURRENT       0x89u
#define INA233_REG_POWER         0x97u
#define INA233_REG_SHUNT_VOLTAGE 0xD1u
#define INA233_REG_CALIBRATION   0xD4u

typedef struct {
	uint16_t bus_voltage_raw;
	uint16_t shunt_voltage_raw;
	uint16_t current_raw;
	uint16_t power_raw;
	uint16_t energy_raw;
	uint8_t energy_rollover;
	uint32_t energy_sample_count;
	float bus_voltage_v;
	float shunt_voltage_mv;
	float current_ma;
	float power_mw;
} ina233_raw_sample_t;

bool ina233_device_init(uint8_t address, float r_shunt, float i_max);
bool ina233_read_register(uint8_t command, uint16_t *value);
bool ina233_read_sample(ina233_raw_sample_t *sample);

#endif
