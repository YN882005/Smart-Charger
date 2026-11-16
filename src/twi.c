#include "twi.h"

#include <avr/io.h>

#include "config.h"

#define TWI_TIMEOUT_LOOPS 60000u

enum {
	TWI_STATUS_START = 0x08u,
	TWI_STATUS_REPEATED_START = 0x10u,
	TWI_STATUS_SLA_W_ACK = 0x18u,
	TWI_STATUS_DATA_W_ACK = 0x28u,
	TWI_STATUS_SLA_R_ACK = 0x40u,
	TWI_STATUS_DATA_R_ACK = 0x50u,
	TWI_STATUS_DATA_R_NACK = 0x58u
};

static bool twi_wait_complete(void)
{
	uint16_t timeout = TWI_TIMEOUT_LOOPS;

	while ((TWCR & _BV(TWINT)) == 0u && timeout > 0u) {
		--timeout;
	}

	return timeout > 0u;
}

static uint8_t twi_status(void)
{
	return (uint8_t)(TWSR & 0xF8u);
}

static bool twi_start(void)
{
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	return twi_wait_complete()
		&& (twi_status() == TWI_STATUS_START
			|| twi_status() == TWI_STATUS_REPEATED_START);
}

static bool twi_write(uint8_t value, uint8_t expected_status)
{
	TWDR = value;
	TWCR = _BV(TWINT) | _BV(TWEN);
	return twi_wait_complete() && twi_status() == expected_status;
}

static bool twi_read(uint8_t *value, bool acknowledge, uint8_t expected_status)
{
	TWCR = _BV(TWINT) | _BV(TWEN) | (acknowledge ? _BV(TWEA) : 0u);
	if (!twi_wait_complete() || twi_status() != expected_status) {
		return false;
	}

	*value = TWDR;
	return true;
}

static void twi_stop(void)
{
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
}

void twi_init(void)
{
	TWSR = 0;
	TWBR = (uint8_t)(((F_CPU / TWI_CLOCK_HZ) - 16UL) / 2UL);
	TWCR = _BV(TWEN);
}

bool twi_read_register_word(uint8_t address, uint8_t command, uint16_t *value)
{
	uint8_t low;
	uint8_t high;
	bool ok = false;

	if (value == 0 || address > 0x7Fu) {
		return false;
	}

	if (!twi_start()
		|| !twi_write((uint8_t)(address << 1), TWI_STATUS_SLA_W_ACK)
		|| !twi_write(command, TWI_STATUS_DATA_W_ACK)
		|| !twi_start()
		|| !twi_write((uint8_t)((address << 1) | 1u), TWI_STATUS_SLA_R_ACK)
		|| !twi_read(&low, true, TWI_STATUS_DATA_R_ACK)
		|| !twi_read(&high, false, TWI_STATUS_DATA_R_NACK)) {
		goto cleanup;
	}

	*value = (uint16_t)low | ((uint16_t)high << 8);
	ok = true;

cleanup:
	twi_stop();
	return ok;
}
