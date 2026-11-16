#include "ina233.h"

#include "twi.h"

bool ina233_device_init(ina233_device_t *device, uint8_t address,
	float r_shunt, float i_max)
{
	float current_lsb;
	float calibration;
	uint16_t calibration_value;

	if (device == 0) {
		return false;
	}
	device->initialized = false;

	if (address > 0x7Fu || !(r_shunt > 0.0f) || !(i_max > 0.0f)) {
		return false;
	}

	current_lsb = i_max / 32768.0f;
	calibration = 0.00512f / (r_shunt * current_lsb);
	if (!(calibration >= 1.0f && calibration <= 65535.0f)) {
		return false;
	}

	calibration_value = (uint16_t)(calibration + 0.5f);
	if (calibration_value == 0u
		|| !twi_write_register_word(address, INA233_REG_CALIBRATION, calibration_value)) {
		return false;
	}

	device->address = address;
	device->current_lsb = current_lsb;
	device->initialized = true;
	return true;
}

static bool ina233_read_register(const ina233_device_t *device, uint8_t command,
	uint16_t *value)
{
	if (device == 0 || !device->initialized || value == 0) {
		return false;
	}

	return twi_read_register_word(device->address, command, value);
}

bool ina233_read_sample(const ina233_device_t *device,
	ina233_raw_sample_t *sample)
{
	ina233_raw_sample_t next = {0};

	if (device == 0 || !device->initialized || sample == 0) {
		return false;
	}

	if (!ina233_read_register(device, INA233_REG_BUS_VOLTAGE, &next.bus_voltage_raw)
		|| !ina233_read_register(device, INA233_REG_CURRENT, &next.current_raw)) {
		return false;
	}

	next.bus_voltage_v = (float)next.bus_voltage_raw / 800.0f;
	next.current_ma = (float)((int16_t)next.current_raw)
		* device->current_lsb * 1000.0f;

	*sample = next;
	return true;
}
