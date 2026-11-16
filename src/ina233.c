#include "ina233.h"

#include "twi.h"

#define INA233_ENERGY_DATA_LENGTH 6u

static uint8_t s_ina233_address;
static float s_current_lsb;
static float s_power_lsb;
static bool s_ina233_initialized;

bool ina233_device_init(uint8_t address, float r_shunt, float i_max)
{
	float current_lsb;
	float power_lsb;
	float calibration;
	uint16_t calibration_value;

	s_ina233_initialized = false;
	twi_init();

	if (address > 0x7Fu || !(r_shunt > 0.0f) || !(i_max > 0.0f)) {
		return false;
	}

	current_lsb = i_max / 32768.0f;
	power_lsb = 25.0f * current_lsb;
	calibration = 0.00512f / (r_shunt * current_lsb);
	if (!(calibration >= 1.0f && calibration <= 65535.0f)) {
		return false;
	}

	calibration_value = (uint16_t)(calibration + 0.5f);
	if (calibration_value == 0u
		|| !twi_write_register_word(address, INA233_REG_CALIBRATION, calibration_value)) {
		return false;
	}

	s_ina233_address = address;
	s_current_lsb = current_lsb;
	s_power_lsb = power_lsb;
	s_ina233_initialized = true;
	return true;
}

bool ina233_read_register(uint8_t command, uint16_t *value)
{
	if (!s_ina233_initialized || value == 0) {
		return false;
	}

	return twi_read_register_word(s_ina233_address, command, value);
}

bool ina233_read_sample(ina233_raw_sample_t *sample)
{
	ina233_raw_sample_t next = {0};
	uint8_t energy_data[INA233_ENERGY_DATA_LENGTH] = {0};
	uint8_t actual_length = 0u;

	if (!s_ina233_initialized || sample == 0) {
		return false;
	}

	if (!ina233_read_register(INA233_REG_BUS_VOLTAGE, &next.bus_voltage_raw)
		|| !ina233_read_register(INA233_REG_SHUNT_VOLTAGE, &next.shunt_voltage_raw)
		|| !ina233_read_register(INA233_REG_CURRENT, &next.current_raw)
		|| !ina233_read_register(INA233_REG_POWER, &next.power_raw)
		|| !twi_read_block(s_ina233_address, INA233_REG_ENERGY,
			energy_data, sizeof(energy_data), &actual_length)
		|| actual_length != sizeof(energy_data)) {
		return false;
	}

	next.energy_raw = (uint16_t)energy_data[0]
		| ((uint16_t)energy_data[1] << 8);
	next.energy_rollover = energy_data[2];
	next.energy_sample_count = (uint32_t)energy_data[3]
		| ((uint32_t)energy_data[4] << 8)
		| ((uint32_t)energy_data[5] << 16);

	next.bus_voltage_v = (float)next.bus_voltage_raw / 800.0f;
	next.shunt_voltage_mv = (float)((int16_t)next.shunt_voltage_raw) / 400.0f;
	next.current_ma = (float)((int16_t)next.current_raw) * s_current_lsb * 1000.0f;
	next.power_mw = (float)((int16_t)next.power_raw) * s_power_lsb * 1000.0f;

	*sample = next;
	return true;
}
