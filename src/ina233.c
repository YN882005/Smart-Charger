#include "ina233.h"

#include "config.h"
#include "twi.h"

bool ina233_read_register(uint8_t command, uint16_t *value)
{
	return twi_read_register_word(INA233_I2C_ADDRESS, command, value);
}

bool ina233_read_sample(ina233_raw_sample_t *sample)
{
	if (sample == 0
		|| !ina233_read_register(INA233_REG_BUS_VOLTAGE, &sample->bus_voltage_raw)
		|| !ina233_read_register(INA233_REG_CURRENT, &sample->current_raw)
		|| !ina233_read_register(INA233_REG_POWER, &sample->power_raw)
		|| !ina233_read_register(INA233_REG_ENERGY, &sample->energy_raw)
		|| !ina233_read_register(INA233_REG_CHARGE, &sample->charge_raw)) {
		return false;
	}

	return true;
}
