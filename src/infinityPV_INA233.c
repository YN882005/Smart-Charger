/**************************************************************************/
/*!
    @file     infinityPV_INA233.c
    @author   rava (infinityPV ApS) / Adapted for Smart Charger
    @license  BSD (see license.txt)

    Implementation of the infinityPV INA233 library for AVR-GCC / baremetal TWI.
*/
/**************************************************************************/

#include "infinityPV_INA233.h"

#include <stdbool.h>
#include <stdint.h>
#include "twi.h"

static float power_of_ten(int8_t exp)
{
	float val = 1.0f;
	if (exp > 0) {
		while (exp > 0) {
			val *= 10.0f;
			--exp;
		}
	} else if (exp < 0) {
		while (exp < 0) {
			val /= 10.0f;
			++exp;
		}
	}
	return val;
}

void ina233_init(ina233_t *dev, uint8_t addr)
{
	if (dev == 0) {
		return;
	}
	dev->address = addr;
	dev->cal_value = 0u;
	dev->m_c = 0;
	dev->R_c = 0;
	dev->m_p = 0;
	dev->R_p = 0;
}

void ina233_begin(ina233_t *dev)
{
	(void)dev;
	twi_init();
}

void ina233_wire_send_cmd(ina233_t *dev, uint8_t reg)
{
	if (dev == 0) {
		return;
	}
	twi_send_command(dev->address, reg);
}

void ina233_wire_write_byte(ina233_t *dev, uint8_t reg, uint8_t value)
{
	if (dev == 0) {
		return;
	}
	twi_write_register_byte(dev->address, reg, value);
}

void ina233_wire_write_word(ina233_t *dev, uint8_t reg, uint16_t value)
{
	if (dev == 0) {
		return;
	}
	twi_write_register_word(dev->address, reg, value);
}

void ina233_wire_read_byte(ina233_t *dev, uint8_t reg, uint8_t *value)
{
	if (dev == 0 || value == 0) {
		return;
	}
	twi_read_register_byte(dev->address, reg, value);
}

void ina233_wire_read_word(ina233_t *dev, uint8_t reg, uint16_t *value)
{
	if (dev == 0 || value == 0) {
		return;
	}
	twi_read_register_word(dev->address, reg, value);
}

void ina233_wire_read_block(ina233_t *dev, uint8_t reg, uint8_t value[6])
{
	uint8_t actual_length = 0u;
	if (dev == 0 || value == 0) {
		return;
	}
	twi_read_block(dev->address, reg, value, 6u, &actual_length);
}

uint16_t ina233_set_calibration(ina233_t *dev, float r_shunt, float i_max,
	float *Current_LSB, float *Power_LSB,
	int16_t *mc, int8_t *Rc, int16_t *mp, int8_t *Rp, uint8_t *ERROR)
{
	float C_LSB = 0.0f;
	float P_LSB = 0.0f;
	float CAL = 0.0f;
	float m_c_F = 0.0f;
	float m_p_F = 0.0f;
	int32_t aux = 0;
	bool round_done = false;
	int8_t local_R_c = 0;
	int8_t local_R_p = 0;
	uint8_t local_ERROR = 0u;

	if (dev == 0 || r_shunt <= 0.0f || i_max <= 0.0f) {
		if (ERROR != 0) {
			*ERROR = 1u;
		}
		return 0u;
	}

	C_LSB = i_max / 32768.0f;
	P_LSB = 25.0f * C_LSB;

	if (Current_LSB != 0) {
		*Current_LSB = C_LSB * 1000000.0f;
	}
	if (Power_LSB != 0) {
		*Power_LSB = P_LSB * 1000.0f;
	}

	CAL = 0.00512f / (r_shunt * C_LSB);

	if (CAL > 65535.0f || CAL < 0.0f) {
		local_ERROR = 1u;
	} else {
		ina233_wire_write_word(dev, MFR_CALIBRATION, (uint16_t)CAL);
	}

	m_c_F = 1.0f / C_LSB;
	m_p_F = 1.0f / P_LSB;

	/* Calculate m and R for maximum accuracy in current measurement */
	aux = (int32_t)m_c_F;
	while (aux > 32767 || aux < -32768) {
		m_c_F /= 10.0f;
		local_R_c++;
		aux = (int32_t)m_c_F;
	}
	while (!round_done) {
		aux = (int32_t)m_c_F;
		if ((float)aux == m_c_F) {
			round_done = true;
		} else {
			aux = (int32_t)(m_c_F * 10.0f);
			if (aux > 32767 || aux < -32768) {
				round_done = true;
			} else {
				m_c_F *= 10.0f;
				local_R_c--;
			}
		}
	}

	round_done = false;
	/* Calculate m and R for maximum accuracy in power measurement */
	aux = (int32_t)m_p_F;
	while (aux > 32767 || aux < -32768) {
		m_p_F /= 10.0f;
		local_R_p++;
		aux = (int32_t)m_p_F;
	}
	while (!round_done) {
		aux = (int32_t)m_p_F;
		if ((float)aux == m_p_F) {
			round_done = true;
		} else {
			aux = (int32_t)(m_p_F * 10.0f);
			if (aux > 32767 || aux < -32768) {
				round_done = true;
			} else {
				m_p_F *= 10.0f;
				local_R_p--;
			}
		}
	}

	if (mp != 0) {
		*mp = (int16_t)m_p_F;
	}
	if (mc != 0) {
		*mc = (int16_t)m_c_F;
	}
	if (Rc != 0) {
		*Rc = local_R_c;
	}
	if (Rp != 0) {
		*Rp = local_R_p;
	}
	if (ERROR != 0) {
		*ERROR = local_ERROR;
	}

	dev->m_c = (int16_t)m_c_F;
	dev->m_p = (int16_t)m_p_F;
	dev->R_c = local_R_c;
	dev->R_p = local_R_p;
	dev->cal_value = (uint16_t)CAL;

	return (uint16_t)CAL;
}

int16_t ina233_get_bus_voltage_raw(ina233_t *dev)
{
	uint16_t value = 0u;
	ina233_wire_read_word(dev, READ_VIN, &value);
	return (int16_t)value;
}

int16_t ina233_get_shunt_voltage_raw(ina233_t *dev)
{
	uint16_t value = 0u;
	ina233_wire_read_word(dev, MFR_READ_VSHUNT, &value);
	return (int16_t)value;
}

int16_t ina233_get_current_raw(ina233_t *dev)
{
	uint16_t value = 0u;
	ina233_wire_read_word(dev, READ_IIN, &value);
	return (int16_t)value;
}

int16_t ina233_get_power_raw(ina233_t *dev)
{
	uint16_t value = 0u;
	ina233_wire_read_word(dev, READ_PIN, &value);
	return (int16_t)value;
}

void ina233_get_energy_raw(ina233_t *dev, uint16_t *accumulator, uint8_t *roll_over, uint32_t *sample_count)
{
	uint8_t value[6] = {0};
	ina233_wire_read_block(dev, READ_EIN, value);
	if (accumulator != 0) {
		*accumulator = (uint16_t)value[0] | ((uint16_t)value[1] << 8);
	}
	if (roll_over != 0) {
		*roll_over = value[2];
	}
	if (sample_count != 0) {
		*sample_count = (uint32_t)value[3]
			| ((uint32_t)value[4] << 8)
			| ((uint32_t)value[5] << 16);
	}
}

float ina233_get_bus_voltage_v(ina233_t *dev)
{
	int16_t raw = ina233_get_bus_voltage_raw(dev);
	/* Vbus (V) = (value * 10^-2 - 0) / 8 = value / 800.0f */
	return (float)raw / 800.0f;
}

float ina233_get_shunt_voltage_mv(ina233_t *dev)
{
	int16_t raw = ina233_get_shunt_voltage_raw(dev);
	/* Vshunt (mV) = ((value * 10^-5 - 0) / 4) * 1000 = value / 400.0f */
	return (float)raw / 400.0f;
}

float ina233_get_current_ma(ina233_t *dev)
{
	int16_t raw;
	float current;

	if (dev == 0 || dev->m_c == 0) {
		return 0.0f;
	}
	raw = ina233_get_current_raw(dev);
	current = (((float)raw * power_of_ten((int8_t)(-dev->R_c))) - (float)b_c) / (float)dev->m_c;
	return current * 1000.0f;
}

float ina233_get_power_mw(ina233_t *dev)
{
	int16_t raw;
	float power;

	if (dev == 0 || dev->m_p == 0) {
		return 0.0f;
	}
	raw = ina233_get_power_raw(dev);
	power = (((float)raw * power_of_ten((int8_t)(-dev->R_p))) - (float)b_p) / (float)dev->m_p;
	return power * 1000.0f;
}

float ina233_get_av_power_mw(ina233_t *dev)
{
	uint16_t accumulator = 0u;
	uint8_t roll_over = 0u;
	uint32_t sample_count = 0u;
	uint32_t accumulator_24;
	uint32_t raw_av_power;
	float av_power;

	if (dev == 0 || dev->m_p == 0) {
		return 0.0f;
	}

	ina233_get_energy_raw(dev, &accumulator, &roll_over, &sample_count);
	if (sample_count == 0u) {
		return 0.0f;
	}

	accumulator_24 = ((uint32_t)roll_over * 65536UL) + (uint32_t)accumulator;
	raw_av_power = accumulator_24 / sample_count;

	av_power = (((float)raw_av_power * power_of_ten((int8_t)(-dev->R_p))) - (float)b_p) / (float)dev->m_p;
	return av_power * 1000.0f;
}
