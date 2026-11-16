#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "buzzer.h"
#include "Charging.h"
#include "Discharging.h"
#include "encoder.h"
#include "OLED_Screen.h"
#include "config.h"
#include "ina233.h"
#include "twi.h"

volatile ina233_raw_sample_t g_ina233_samples[INA233_CHANNEL_COUNT];
volatile bool g_ina233_sample_valid[INA233_CHANNEL_COUNT];
volatile bool g_oled_ready;

static void refresh_samples(ina233_device_t devices[INA233_CHANNEL_COUNT],
	ina233_raw_sample_t samples[INA233_CHANNEL_COUNT],
	const bool device_ready[INA233_CHANNEL_COUNT],
	bool sample_valid[INA233_CHANNEL_COUNT])
{
	uint8_t channel;

	for (channel = 0u; channel < (uint8_t)INA233_CHANNEL_COUNT; ++channel) {
		sample_valid[channel] = device_ready[channel]
			&& ina233_read_sample(&devices[channel], &samples[channel]);
		g_ina233_sample_valid[channel] = sample_valid[channel];
		if (sample_valid[channel]) {
			g_ina233_samples[channel] = samples[channel];
		}
	}
}

int main(void)
{
	ina233_device_t devices[INA233_CHANNEL_COUNT] = {{0}};
	ina233_raw_sample_t samples[INA233_CHANNEL_COUNT] = {{0}};
	bool device_ready[INA233_CHANNEL_COUNT] = {false, false};
	bool sample_valid[INA233_CHANNEL_COUNT] = {false, false};
	uint8_t measurement_ticks = 0u;
	uint8_t display_ticks = 0u;
	bool alert_active;

	charging_init_safe();
	discharging_init_safe();
	encoder_init();
	buzzer_init();

	INA233_ALERT_DDR &= (uint8_t)~_BV(INA233_ALERT_BIT);
	INA233_ALERT_PORT |= _BV(INA233_ALERT_BIT);
	twi_init();
	device_ready[0] = ina233_device_init(&devices[0], INA233_CH1_I2C_ADDRESS,
		INA233_R_SHUNT_OHMS, INA233_I_MAX_AMPS);
	device_ready[1] = ina233_device_init(&devices[1], INA233_CH2_I2C_ADDRESS,
		INA233_R_SHUNT_OHMS, INA233_I_MAX_AMPS);
	refresh_samples(devices, samples, device_ready, sample_valid);

	g_oled_ready = oled_screen_init();
	if (g_oled_ready) {
		alert_active = (INA233_ALERT_PIN & _BV(INA233_ALERT_BIT)) == 0u;
		g_oled_ready = oled_screen_render(sample_valid, samples, alert_active);
	}
	sei();

	for (;;) {
		encoder_event_t event = encoder_poll();

		if (event != ENCODER_EVENT_NONE) {
			oled_screen_handle_encoder_event(event);
		}

		if (++measurement_ticks >= 100u) {
			measurement_ticks = 0u;
			refresh_samples(devices, samples, device_ready, sample_valid);
			charging_disable();
			discharging_disable();
		}

		if (++display_ticks >= 50u) {
			display_ticks = 0u;
			alert_active = (INA233_ALERT_PIN & _BV(INA233_ALERT_BIT)) == 0u;
			g_oled_ready = oled_screen_render(sample_valid, samples, alert_active);
		}

		_delay_ms(10);
	}
}
