#include "Encoder.h"

#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "config.h"

static volatile int8_t encoder_delta;
static volatile uint8_t encoder_prev_state;
static uint8_t switch_integrator;
static bool switch_latched;

ISR(PCINT1_vect)
{
	static const int8_t transitions[] = {
		0, -1, 1, 0,
		1, 0, 0, -1,
		-1, 0, 0, 1,
		0, 1, -1, 0
	};
	uint8_t pins = PINC;
	uint8_t state = (uint8_t)(((pins >> ENCODER_DT_BIT) & 0x01u) << 1)
		| ((pins >> ENCODER_CLK_BIT) & 0x01u);
	uint8_t index = (uint8_t)((encoder_prev_state << 2) | state);

	encoder_delta = (int8_t)(encoder_delta + transitions[index & 0x0Fu]);
	encoder_prev_state = state;
}

void encoder_init(void)
{
	/* Inputs with pull-ups for HW-040 rotary encoder. */
	DDRC &= (uint8_t)~(_BV(ENCODER_CLK_BIT) | _BV(ENCODER_DT_BIT));
	PORTC |= _BV(ENCODER_CLK_BIT) | _BV(ENCODER_DT_BIT);

	DDRD &= (uint8_t)~_BV(ENCODER_SWITCH_BIT);
	PORTD |= _BV(ENCODER_SWITCH_BIT);

	encoder_prev_state = (uint8_t)(((PINC >> ENCODER_DT_BIT) & 0x01u) << 1)
		| ((PINC >> ENCODER_CLK_BIT) & 0x01u);
	encoder_delta = 0;
	switch_integrator = 0u;
	switch_latched = false;

	PCICR |= _BV(PCIE1);
	PCMSK1 |= _BV(PCINT9) | _BV(PCINT10);
}

encoder_event_t encoder_poll(void)
{
	int8_t delta;
	bool pressed;

	cli();
	delta = encoder_delta;
	encoder_delta = 0;
	sei();

	if (delta >= 2) {
		return ENCODER_EVENT_CW;
	}
	if (delta <= -2) {
		return ENCODER_EVENT_CCW;
	}

	pressed = (PIND & _BV(ENCODER_SWITCH_BIT)) == 0u;
	if (pressed) {
		if (switch_integrator < 5u) {
			++switch_integrator;
		}
	} else if (switch_integrator > 0u) {
		--switch_integrator;
	}

	if (switch_integrator >= 4u) {
		if (!switch_latched) {
			switch_latched = true;
			return ENCODER_EVENT_PRESS;
		}
	} else if (switch_integrator == 0u) {
		switch_latched = false;
	}

	return ENCODER_EVENT_NONE;
}
