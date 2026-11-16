#include "buzzer.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "config.h"

#define BUZZER_TIMER_PRESCALER 32UL
#define BUZZER_COMPARE_VALUE \
	((F_CPU / (2UL * BUZZER_TIMER_PRESCALER * BUZZER_FREQUENCY_HZ)) - 1UL)

static volatile uint16_t half_cycles_remaining;

ISR(TIMER2_COMPA_vect)
{
	if (half_cycles_remaining > 0u) {
		PIND = _BV(BUZZER_BIT);
		--half_cycles_remaining;
	}

	if (half_cycles_remaining == 0u) {
		TCCR2B = 0;
		TIMSK2 &= (uint8_t)~_BV(OCIE2A);
		PORTD &= (uint8_t)~_BV(BUZZER_BIT);
	}
}

void buzzer_init(void)
{
	PORTD &= (uint8_t)~_BV(BUZZER_BIT);
	DDRD |= _BV(BUZZER_BIT);

	TCCR2A = _BV(WGM21);
	TCCR2B = 0;
	OCR2A = (uint8_t)BUZZER_COMPARE_VALUE;
	TCNT2 = 0;
	TIMSK2 &= (uint8_t)~_BV(OCIE2A);
}

void buzzer_beep(uint16_t duration_ms)
{
	uint32_t cycles;

	if (duration_ms == 0u) {
		return;
	}

	cycles = (BUZZER_FREQUENCY_HZ * (uint32_t)duration_ms * 2UL) / 1000UL;
	if (cycles == 0u) {
		cycles = 1u;
	} else if (cycles > 65535UL) {
		cycles = 65535UL;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		half_cycles_remaining = (uint16_t)cycles;
		PORTD &= (uint8_t)~_BV(BUZZER_BIT);
		TCNT2 = 0;
		TIFR2 = _BV(OCF2A);
		TIMSK2 |= _BV(OCIE2A);
		TCCR2B = _BV(CS21) | _BV(CS20);
	}
}
