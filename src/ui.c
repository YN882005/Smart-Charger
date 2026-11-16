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

static void format_float_value(char *line, const char *label, float val, const char *unit)
{
	uint8_t index = 0u;
	int32_t int_part;
	int32_t frac_part;
	char int_buf[10];
	uint8_t int_idx = 0u;

	while (label[index] != '\0' && index < 14u) {
		line[index] = label[index];
		++index;
	}

	if (val < 0.0f) {
		line[index++] = '-';
		val = -val;
	}

	int_part = (int32_t)val;
	frac_part = (int32_t)((val - (float)int_part) * 100.0f + 0.5f);
	if (frac_part >= 100) {
		int_part += 1;
		frac_part -= 100;
	}

	if (int_part == 0) {
		int_buf[int_idx++] = '0';
	} else {
		while (int_part > 0 && int_idx < sizeof(int_buf)) {
			int_buf[int_idx++] = (char)('0' + (int_part % 10));
			int_part /= 10;
		}
	}

	while (int_idx > 0u && index < 16u) {
		line[index++] = int_buf[--int_idx];
	}

	if (index < 18u) {
		line[index++] = '.';
		line[index++] = (char)('0' + (frac_part / 10));
		line[index++] = (char)('0' + (frac_part % 10));
	}

	while (*unit != '\0' && index < 21u) {
		line[index++] = *unit++;
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
		if (screen < 2u) {
			screen++;
		}
	} else if (event == ENCODER_EVENT_CCW) {
		if (screen > 0u) {
			screen--;
		}
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
	} else if (screen == 1u) {
		ok = ssd1306_write_line(0u, "INA233 DATA") && ok;
		if (sample_valid && sample != 0) {
			format_float_value(line, "VBUS: ", sample->bus_voltage_v, " V");
			ok = ssd1306_write_line(1u, line) && ok;
			format_float_value(line, "VSHU: ", sample->shunt_voltage_mv, " mV");
			ok = ssd1306_write_line(2u, line) && ok;
			format_float_value(line, "CURR: ", sample->current_ma, " mA");
			ok = ssd1306_write_line(3u, line) && ok;
			format_float_value(line, "POW:  ", sample->power_mw, " mW");
			ok = ssd1306_write_line(4u, line) && ok;
			ok = ssd1306_write_line(5u, "PAGE 2: RAW") && ok;
		} else {
			ok = ssd1306_write_line(1u, "SENSOR ERROR") && ok;
			ok = ssd1306_write_line(2u, "") && ok;
			ok = ssd1306_write_line(3u, "") && ok;
			ok = ssd1306_write_line(4u, "") && ok;
			ok = ssd1306_write_line(5u, "") && ok;
		}
	} else {
		ok = ssd1306_write_line(0u, "INA233 RAW") && ok;
		if (sample_valid && sample != 0) {
			format_raw_value(line, "VBUS:", sample->bus_voltage_raw);
			ok = ssd1306_write_line(1u, line) && ok;
			format_raw_value(line, "VSHU:", sample->shunt_voltage_raw);
			ok = ssd1306_write_line(2u, line) && ok;
			format_raw_value(line, "CURR:", sample->current_raw);
			ok = ssd1306_write_line(3u, line) && ok;
			format_raw_value(line, "POW:", sample->power_raw);
			ok = ssd1306_write_line(4u, line) && ok;
			format_raw_value(line, "ENGY:", sample->energy_raw);
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
