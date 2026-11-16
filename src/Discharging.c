#include "Discharging.h"

#include <avr/io.h>
#include <util/atomic.h>

#include "config.h"

#define DISCHARGE_PORTB_MASK \
	(_BV(DISCHARGE1_BIT) | _BV(DISCHARGE2_BIT) | _BV(DISCHARGE3_BIT))

void discharging_disable(void)
{
	/* Inactive state: discharge control pins held low */
	PORTB &= (uint8_t)~DISCHARGE_PORTB_MASK;
	PORTC &= (uint8_t)~_BV(DISCHARGE4_BIT);
}

void discharging_init_safe(void)
{
	/* Establish inactive levels before enabling drivers */
	PORTB &= (uint8_t)~DISCHARGE_PORTB_MASK;
	DDRB |= DISCHARGE_PORTB_MASK;

	PORTC &= (uint8_t)~_BV(DISCHARGE4_BIT);
	DDRC |= _BV(DISCHARGE4_BIT);

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		discharging_disable();
	}
}

bool discharging_outputs_are_safe(void)
{
	return (PORTB & DISCHARGE_PORTB_MASK) == 0u
		&& (PORTC & _BV(DISCHARGE4_BIT)) == 0u;
}
