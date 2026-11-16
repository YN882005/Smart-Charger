#include "encoder.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>

#include "config.h"

static const int8_t transition_delta[16] PROGMEM = {
	0, -1, 1, 0,
	1, 0, 0, -1,
	-1, 0, 0, 1,
	0, 1, -1, 0
};

static volatile int8_t quarter_steps;
static volatile int8_t pending_steps;
static uint8_t previous_state;
static uint8_t button_integrator;
static uint8_t button_pressed;

static uint8_t encoder_state(void)
{
	uint8_t pins = PINC;
	return (uint8_t)(((pins & _BV(ENCODER_CLK_BIT)) != 0u ? 2u : 0u)
		| ((pins & _BV(ENCODER_DT_BIT)) != 0u ? 1u : 0u));
}

ISR(PCINT1_vect)
{
	uint8_t current_state = encoder_state();
	uint8_t transition = (uint8_t)((previous_state << 2) | current_state);
	int8_t delta = (int8_t)pgm_read_byte(&transition_delta[transition]);

	previous_state = current_state;
	quarter_steps += delta;

	if (quarter_steps >= 4) {
		quarter_steps -= 4;
		if (pending_steps < 100) {
			++pending_steps;
		}
	} else if (quarter_steps <= -4) {
		quarter_steps += 4;
		if (pending_steps > -100) {
			--pending_steps;
		}
	}
}

void encoder_init(void)
{
	DDRC &= (uint8_t)~(_BV(ENCODER_CLK_BIT) | _BV(ENCODER_DT_BIT));
	PORTC |= _BV(ENCODER_CLK_BIT) | _BV(ENCODER_DT_BIT);
	DDRD &= (uint8_t)~_BV(ENCODER_SWITCH_BIT);
	PORTD |= _BV(ENCODER_SWITCH_BIT);

	previous_state = encoder_state();
	quarter_steps = 0;
	pending_steps = 0;
	button_integrator = 0;
	button_pressed = 0;

	PCMSK1 |= _BV(PCINT9) | _BV(PCINT10);
	PCIFR = _BV(PCIF1);
	PCICR |= _BV(PCIE1);
}

encoder_event_t encoder_poll(void)
{
	int8_t steps;
	uint8_t button_down = (PIND & _BV(ENCODER_SWITCH_BIT)) == 0u;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		steps = pending_steps;
		pending_steps = 0;
	}

	if (steps > 0) {
		return ENCODER_EVENT_CW;
	}
	if (steps < 0) {
		return ENCODER_EVENT_CCW;
	}

	if (button_down) {
		if (button_integrator < 4u) {
			++button_integrator;
		}
		if (button_integrator == 4u && button_pressed == 0u) {
			button_pressed = 1u;
			return ENCODER_EVENT_PRESS;
		}
	} else {
		if (button_integrator > 0u) {
			--button_integrator;
		}
		if (button_integrator == 0u) {
			button_pressed = 0u;
		}
	}

	return ENCODER_EVENT_NONE;
}
