#include "board.h"

#include <avr/io.h>
#include <util/atomic.h>

#include "config.h"

void board_charge_disable(void)
{
	TCCR1A &= (uint8_t)~(_BV(COM1A1) | _BV(COM1A0));
	PORTB |= _BV(CHARGE_PWM_BIT);
}

void board_discharge_disable(void)
{
	PORTD &= (uint8_t)~_BV(DISCHARGE_GATE_BIT);
}

void board_outputs_safe(void)
{
	board_charge_disable();
	board_discharge_disable();
}

void board_init_safe(void)
{
	/* Establish inactive output levels before enabling the output drivers. */
	PORTB |= _BV(CHARGE_PWM_BIT);
	DDRB |= _BV(CHARGE_PWM_BIT);
	PORTD &= (uint8_t)~_BV(DISCHARGE_GATE_BIT);
	DDRD |= _BV(DISCHARGE_GATE_BIT);

	TCCR1A = 0;
	TCCR1B = 0;
	TIMSK1 = 0;
	OCR1A = 0;
	TCNT1 = 0;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		board_outputs_safe();
	}
}

bool board_outputs_are_safe(void)
{
	return (PORTB & _BV(CHARGE_PWM_BIT)) != 0u
		&& (TCCR1A & (_BV(COM1A1) | _BV(COM1A0))) == 0u
		&& (PORTD & _BV(DISCHARGE_GATE_BIT)) == 0u;
}
