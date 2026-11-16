#include "ui.h"

#include <stdint.h>

#include "buzzer.h"
#include "ssd1306.h"

static bool display_ready;
static uint8_t screen;

static void format_raw_value(char *line, const char *label, uint16_t value)
{
	static const char hex[] = "0123456789ABCDEF";
	uint8_t index = 0u;
	uint8_t digit;

	while (label[index] != '\0') {
		line[index] = label[index];
		++index;
	}

	line[index++] = '0';
	line[index++] = 'X';
	for (digit = 0u; digit < 4u; ++digit) {
		uint8_t shift = (uint8_t)(12u - (digit * 4u));
		line[index++] = hex[(value >> shift) & 0x0Fu];
	}
	line[index] = '\0';
}

bool ui_init(void)
{
	screen = 0u;
	display_ready = ssd1306_init();
	return display_ready;
}

void ui_handle_encoder_event(encoder_event_t event)
{
	if (event == ENCODER_EVENT_CW) {
		screen = 1u;
	} else if (event == ENCODER_EVENT_CCW) {
		screen = 0u;
	} else if (event == ENCODER_EVENT_PRESS) {
		buzzer_beep(45u);
	}
}

bool ui_render(bool sample_valid, const ina233_raw_sample_t *sample)
{
	char line[22];
	bool ok = true;

	if (!display_ready) {
		return false;
	}

	if (screen == 0u) {
		ok = ssd1306_write_line(0u, "SMART CHARGER") && ok;
		ok = ssd1306_write_line(1u, "MONITOR ONLY") && ok;
		ok = ssd1306_write_line(2u, "CHG ALL OFF") && ok;
		ok = ssd1306_write_line(3u, "DSG ALL OFF") && ok;
		ok = ssd1306_write_line(4u, sample_valid ? "INA233: OK" : "INA233: ERROR") && ok;
		ok = ssd1306_write_line(5u, "TURN FOR DATA") && ok;
	} else {
		ok = ssd1306_write_line(0u, "INA233 RAW") && ok;
		if (sample_valid && sample != 0) {
			format_raw_value(line, "VBUS:", sample->bus_voltage_raw);
			ok = ssd1306_write_line(1u, line) && ok;
			format_raw_value(line, "CURRENT:", sample->current_raw);
			ok = ssd1306_write_line(2u, line) && ok;
			format_raw_value(line, "POWER:", sample->power_raw);
			ok = ssd1306_write_line(3u, line) && ok;
			format_raw_value(line, "ENERGY:", sample->energy_raw);
			ok = ssd1306_write_line(4u, line) && ok;
			format_raw_value(line, "CHARGE:", sample->charge_raw);
			ok = ssd1306_write_line(5u, line) && ok;
		} else {
			ok = ssd1306_write_line(1u, "SENSOR ERROR") && ok;
			ok = ssd1306_write_line(2u, "") && ok;
			ok = ssd1306_write_line(3u, "") && ok;
			ok = ssd1306_write_line(4u, "") && ok;
			ok = ssd1306_write_line(5u, "") && ok;
		}
	}

	if (!ok) {
		display_ready = false;
	}
	return ok;
}
