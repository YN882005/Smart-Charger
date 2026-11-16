#include "Charging.h"

#include <avr/io.h>
#include <util/atomic.h>

#include "config.h"

#define CHARGE_PORTB_MASK \
	(_BV(CHARGE1_BIT) | _BV(CHARGE2_BIT) | _BV(CHARGE3_BIT))

void charging_disable(void)
{
	/* Disconnect all timer outputs before forcing inactive pin levels. */
	TCCR1A &= (uint8_t)~(_BV(COM1A1) | _BV(COM1A0) | _BV(COM1B1) | _BV(COM1B0));
	TCCR2A &= (uint8_t)~(_BV(COM2A1) | _BV(COM2A0) | _BV(COM2B1) | _BV(COM2B0));

	/* Inactive state: charge control pins held high */
	PORTB |= CHARGE_PORTB_MASK;
	PORTD |= _BV(CHARGE4_BIT);
}

void charging_init_safe(void)
{
	/* Establish inactive levels before enabling drivers */
	PORTB |= CHARGE_PORTB_MASK;
	DDRB |= CHARGE_PORTB_MASK;

	PORTD |= _BV(CHARGE4_BIT);
	DDRD |= _BV(CHARGE4_BIT);

	TCCR1A = 0;
	TCCR1B = 0;
	TIMSK1 = 0;
	OCR1A = 0;
	OCR1B = 0;
	TCNT1 = 0;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		charging_disable();
	}
}

bool charging_outputs_are_safe(void)
{
	return (PORTB & CHARGE_PORTB_MASK) == CHARGE_PORTB_MASK
		&& (PORTD & _BV(CHARGE4_BIT)) != 0u
		&& (TCCR1A & (_BV(COM1A1) | _BV(COM1A0)
			| _BV(COM1B1) | _BV(COM1B0))) == 0u
		&& (TCCR2A & (_BV(COM2A1) | _BV(COM2A0)
			| _BV(COM2B1) | _BV(COM2B0))) == 0u;
}
