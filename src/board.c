#include "board.h"

#include <avr/io.h>
#include <util/atomic.h>

#include "config.h"

#define CHARGE_PORTB_MASK \
	(_BV(CHARGE1_BIT) | _BV(CHARGE2_BIT) | _BV(CHARGE3_BIT))
#define DISCHARGE_PORTB_MASK \
	(_BV(DISCHARGE1_BIT) | _BV(DISCHARGE2_BIT) | _BV(DISCHARGE3_BIT))
#define POWER_PORTB_MASK (CHARGE_PORTB_MASK | DISCHARGE_PORTB_MASK)

void board_charge_disable(void)
{
	TCCR1A &= (uint8_t)~(_BV(COM1A1) | _BV(COM1A0));
	PORTB |= CHARGE_PORTB_MASK;
	PORTD |= _BV(CHARGE4_BIT);
}

void board_discharge_disable(void)
{
	PORTB &= (uint8_t)~DISCHARGE_PORTB_MASK;
	PORTC &= (uint8_t)~_BV(DISCHARGE4_BIT);
}

void board_outputs_safe(void)
{
	board_charge_disable();
	board_discharge_disable();
}

void board_init_safe(void)
{
	/* Establish inactive levels before enabling any power-stage output drivers. */
	PORTB = (PORTB & (uint8_t)~POWER_PORTB_MASK) | CHARGE_PORTB_MASK;
	DDRB |= POWER_PORTB_MASK;

	PORTC = (PORTC & (uint8_t)~_BV(DISCHARGE4_BIT))
		| _BV(ENCODER_CLK_BIT) | _BV(ENCODER_DT_BIT);
	DDRC = (DDRC | _BV(DISCHARGE4_BIT))
		& (uint8_t)~(_BV(ENCODER_CLK_BIT) | _BV(ENCODER_DT_BIT));

	PORTD = (PORTD & (uint8_t)~_BV(BUZZER_BIT))
		| _BV(CHARGE4_BIT) | _BV(ENCODER_SWITCH_BIT);
	DDRD = (DDRD | _BV(CHARGE4_BIT) | _BV(BUZZER_BIT))
		& (uint8_t)~_BV(ENCODER_SWITCH_BIT);

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
	return (PORTB & CHARGE_PORTB_MASK) == CHARGE_PORTB_MASK
		&& (PORTB & DISCHARGE_PORTB_MASK) == 0u
		&& (PORTC & _BV(DISCHARGE4_BIT)) == 0u
		&& (PORTD & _BV(CHARGE4_BIT)) != 0u
		&& (PORTD & _BV(BUZZER_BIT)) == 0u
		&& (TCCR1A & (_BV(COM1A1) | _BV(COM1A0))) == 0u;
}
