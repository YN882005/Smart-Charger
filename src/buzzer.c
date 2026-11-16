#include "Buzzer.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include "config.h"

static volatile uint8_t beep_ticks;

ISR(TIMER2_COMPA_vect)
{
	if (beep_ticks > 0u) {
		--beep_ticks;
		if (beep_ticks == 0u) {
			TCCR2A &= (uint8_t)~_BV(COM2B0);
			PORTD &= (uint8_t)~_BV(BUZZER_BIT);
		}
	}
}

void buzzer_init(void)
{
	DDRD |= _BV(BUZZER_BIT);
	PORTD &= (uint8_t)~_BV(BUZZER_BIT);

	/* Mode 2 (CTC), toggle OC2B/PD4, /64 prescaler (~2.3 kHz target). */
	TCCR2A = _BV(WGM21);
	TCCR2B = _BV(CS22);
	OCR2A = (uint8_t)(((F_CPU / 64UL) / (2UL * BUZZER_FREQUENCY_HZ)) - 1UL);
	OCR2B = OCR2A;
	TCNT2 = 0;
	TIMSK2 |= _BV(OCIE2A);
}

void buzzer_beep(uint8_t duration_ms)
{
	uint8_t ticks;

	if (duration_ms == 0u) {
		return;
	}

	ticks = (uint8_t)(((uint32_t)duration_ms * BUZZER_FREQUENCY_HZ * 2UL) / 1000UL);
	if (ticks == 0u) {
		ticks = 1u;
	}

	cli();
	beep_ticks = ticks;
	TCCR2A |= _BV(COM2B0);
	sei();
}
