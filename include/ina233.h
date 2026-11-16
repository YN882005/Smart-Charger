#ifndef INA233_H
#define INA233_H

#include <stdbool.h>
#include <stdint.h>

#define INA233_CHANNEL_COUNT 2u

#define INA233_REG_BUS_VOLTAGE   0x88u
#define INA233_REG_CURRENT       0x89u
#define INA233_REG_CALIBRATION   0xD4u

typedef struct {
	uint16_t bus_voltage_raw;
	uint16_t current_raw;
	float bus_voltage_v;
	float current_ma;
} ina233_raw_sample_t;

typedef struct {
	uint8_t address;
	float current_lsb;
	bool initialized;
} ina233_device_t;

bool ina233_device_init(ina233_device_t *device, uint8_t address,
	float r_shunt, float i_max);
bool ina233_read_sample(const ina233_device_t *device,
	ina233_raw_sample_t *sample);

#endif
