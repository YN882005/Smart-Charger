#include "encoder.h"

#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "config.h"

#define ENCODER_BUTTON_DEBOUNCE 4u
#define ENCODER_DOUBLE_CLICK_TICKS 40u

static volatile int8_t encoder_delta;
static volatile uint8_t encoder_prev_state;
static uint8_t switch_integrator;
static uint8_t pending_click_ticks;
static bool switch_latched;
static bool first_press_pending;

ISR(PCINT1_vect)
{
	static const int8_t transitions[] = {
		0, -1, 1, 0,
		1, 0, 0, -1,
		-1, 0, 0, 1,
		0, 1, -1, 0
	};
	uint8_t pins = ENCODER_PIN;
	uint8_t state = (uint8_t)(((pins >> ENCODER_DT_BIT) & 0x01u) << 1)
		| ((pins >> ENCODER_CLK_BIT) & 0x01u);
	uint8_t index = (uint8_t)((encoder_prev_state << 2) | state);

	encoder_delta = (int8_t)(encoder_delta + transitions[index & 0x0Fu]);
	encoder_prev_state = state;
}

void encoder_init(void)
{
	/* Inputs with pull-ups for HW-040 rotary encoder. */
	ENCODER_DDR &= (uint8_t)~(_BV(ENCODER_CLK_BIT) | _BV(ENCODER_DT_BIT)
		| _BV(ENCODER_SWITCH_BIT));
	ENCODER_PORT |= _BV(ENCODER_CLK_BIT) | _BV(ENCODER_DT_BIT)
		| _BV(ENCODER_SWITCH_BIT);

	encoder_prev_state = (uint8_t)(((ENCODER_PIN >> ENCODER_DT_BIT) & 0x01u) << 1)
		| ((ENCODER_PIN >> ENCODER_CLK_BIT) & 0x01u);
	encoder_delta = 0;
	switch_integrator = 0u;
	pending_click_ticks = 0u;
	switch_latched = false;
	first_press_pending = false;

	PCICR |= _BV(PCIE1);
	PCMSK1 |= _BV(PCINT9) | _BV(PCINT10);
}

encoder_event_t encoder_poll(void)
{
	int8_t delta;
	encoder_event_t rotation_event = ENCODER_EVENT_NONE;
	bool pressed;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		delta = encoder_delta;
		encoder_delta = 0;
	}

	if (delta >= 2) {
		rotation_event = ENCODER_EVENT_CW;
	} else if (delta <= -2) {
		rotation_event = ENCODER_EVENT_CCW;
	}

	if (first_press_pending) {
		if (pending_click_ticks < ENCODER_DOUBLE_CLICK_TICKS) {
			++pending_click_ticks;
		} else {
			first_press_pending = false;
		}
	}

	pressed = (ENCODER_PIN & _BV(ENCODER_SWITCH_BIT)) == 0u;
	if (pressed) {
		if (switch_integrator < 5u) {
			++switch_integrator;
		}
	} else if (switch_integrator > 0u) {
		--switch_integrator;
	}

	if (switch_integrator >= ENCODER_BUTTON_DEBOUNCE) {
		if (!switch_latched) {
			switch_latched = true;
			if (first_press_pending) {
				first_press_pending = false;
				return ENCODER_EVENT_DOUBLE_PRESS;
			}
			first_press_pending = true;
			pending_click_ticks = 0u;
			return ENCODER_EVENT_PRESS;
		}
	} else if (switch_integrator == 0u) {
		switch_latched = false;
	}

	return rotation_event;
}
