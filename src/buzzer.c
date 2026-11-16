#include "buzzer.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "config.h"

#define BUZZER_TIMER_PRESCALER 64UL
#define BUZZER_TOGGLE_RATE_HZ (2UL * BUZZER_FREQUENCY_HZ)

static volatile uint32_t s_beep_toggles;

ISR(TIMER2_COMPA_vect)
{
	if (s_beep_toggles > 0u) {
		/* Toggle PD4 */
		BUZZER_PIN = _BV(BUZZER_BIT);
		--s_beep_toggles;
		if (s_beep_toggles == 0u) {
			/* Silence: hold PD4 firmly low */
			PORTD &= (uint8_t)~_BV(BUZZER_BIT);
			TIMSK2 &= (uint8_t)~_BV(OCIE2A);
		}
	}
}

void buzzer_init(void)
{
	/* Configure PD4 as output and initialize LOW */
	BUZZER_DDR |= _BV(BUZZER_BIT);
	BUZZER_PORT &= (uint8_t)~_BV(BUZZER_BIT);

	s_beep_toggles = 0u;

	/* CTC compare events toggle PD4 at twice the requested output frequency. */
	TCCR2A = _BV(WGM21);
	TCCR2B = _BV(CS22);
	OCR2A = (uint8_t)((F_CPU
		/ (BUZZER_TIMER_PRESCALER * BUZZER_TOGGLE_RATE_HZ)) - 1UL);
	OCR2B = 0u;
	TCNT2 = 0u;

	TIFR2 = _BV(OCF2A);
	TIMSK2 &= (uint8_t)~_BV(OCIE2A);
}

void buzzer_beep(uint16_t duration_ms)
{
	uint32_t toggle_count;

	if (duration_ms == 0u) {
		return;
	}

	toggle_count = ((uint32_t)duration_ms * BUZZER_TOGGLE_RATE_HZ) / 1000UL;
	if (toggle_count == 0u) {
		toggle_count = 1u;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		BUZZER_PORT &= (uint8_t)~_BV(BUZZER_BIT);
		s_beep_toggles = toggle_count;
		TIFR2 = _BV(OCF2A);
		TIMSK2 |= _BV(OCIE2A);
	}
}

bool buzzer_is_beeping(void)
{
	bool beeping;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		beeping = (s_beep_toggles > 0u);
	}
	return beeping;
}
