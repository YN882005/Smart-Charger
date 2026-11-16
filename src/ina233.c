#include "ina233.h"

#include "config.h"
#include "twi.h"

static ina233_t s_ina233_dev = {
	.address = INA233_I2C_ADDRESS,
	.cal_value = 0u,
	.m_c = 0,
	.R_c = 0,
	.m_p = 0,
	.R_p = 0
};

void ina233_device_init(uint8_t address, float r_shunt, float i_max)
{
	float c_lsb = 0.0f;
	float p_lsb = 0.0f;
	int16_t mc = 0;
	int16_t mp = 0;
	int8_t rc = 0;
	int8_t rp = 0;
	uint8_t err = 0u;

	ina233_init(&s_ina233_dev, address);
	ina233_begin(&s_ina233_dev);
	ina233_set_calibration(&s_ina233_dev, r_shunt, i_max,
		&c_lsb, &p_lsb, &mc, &rc, &mp, &rp, &err);
}

ina233_t *ina233_get_default_device(void)
{
	return &s_ina233_dev;
}

bool ina233_read_register(uint8_t command, uint16_t *value)
{
	return twi_read_register_word(s_ina233_dev.address, command, value);
}

bool ina233_read_sample(ina233_raw_sample_t *sample)
{
	uint16_t accumulator = 0u;
	uint8_t roll_over = 0u;
	uint32_t sample_count = 0u;

	if (sample == 0) {
		return false;
	}

	if (!ina233_read_register(READ_VIN, &sample->bus_voltage_raw)) {
		return false;
	}

	if (!ina233_read_register(MFR_READ_VSHUNT, &sample->shunt_voltage_raw)) {
		sample->shunt_voltage_raw = 0u;
	}

	if (!ina233_read_register(READ_IIN, &sample->current_raw)) {
		sample->current_raw = 0u;
	}

	if (!ina233_read_register(READ_PIN, &sample->power_raw)) {
		sample->power_raw = 0u;
	}

	ina233_get_energy_raw(&s_ina233_dev, &accumulator, &roll_over, &sample_count);
	sample->energy_raw = accumulator;
	sample->charge_raw = (uint16_t)(sample_count & 0xFFFFu);

	/* Converted physical units from infinityPV_INA233 */
	sample->bus_voltage_v = (float)((int16_t)sample->bus_voltage_raw) / 800.0f;
	sample->shunt_voltage_mv = (float)((int16_t)sample->shunt_voltage_raw) / 400.0f;
	sample->current_ma = ina233_get_current_ma(&s_ina233_dev);
	sample->power_mw = ina233_get_power_mw(&s_ina233_dev);

	return true;
}
