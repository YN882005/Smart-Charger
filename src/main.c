#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "Buzzer.h"
#include "Charging.h"
#include "Discharging.h"
#include "Encoder.h"
#include "OLED Screen.h"
#include "config.h"
#include "ina233.h"
#include "twi.h"

volatile ina233_raw_sample_t g_ina233_sample;
volatile bool g_ina233_sample_valid;
volatile bool g_oled_ready;

int main(void)
{
	ina233_raw_sample_t sample = {0};
	uint8_t measurement_ticks = 0u;
	uint8_t display_ticks = 0u;
	bool sample_valid;

	charging_init_safe();
	discharging_init_safe();
	twi_init();
	encoder_init();
	buzzer_init();
	ina233_device_init(INA233_I2C_ADDRESS, INA233_R_SHUNT_OHMS, INA233_I_MAX_AMPS);

	sample_valid = ina233_read_sample(&sample);
	g_ina233_sample_valid = sample_valid;
	if (sample_valid) {
		g_ina233_sample = sample;
	}

	g_oled_ready = oled_screen_init();
	if (g_oled_ready) {
		g_oled_ready = oled_screen_render(sample_valid, &sample);
	}
	sei();

	for (;;) {
		encoder_event_t event = encoder_poll();

		if (event != ENCODER_EVENT_NONE) {
			oled_screen_handle_encoder_event(event);
		}

		if (++measurement_ticks >= 100u) {
			measurement_ticks = 0u;
			sample_valid = ina233_read_sample(&sample);
			g_ina233_sample_valid = sample_valid;
			if (sample_valid) {
				g_ina233_sample = sample;
			}
			charging_disable();
			discharging_disable();
		}

		if (++display_ticks >= 50u) {
			display_ticks = 0u;
			g_oled_ready = oled_screen_render(g_ina233_sample_valid, &sample);
		}

		_delay_ms(10);
	}
}
