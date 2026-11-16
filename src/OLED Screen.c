#include "OLED Screen.h"

#include <avr/pgmspace.h>
#include <stdint.h>

#include "Buzzer.h"
#include "config.h"
#include "twi.h"

#define SSD1306_PAGE_COUNT 8u
#define SSD1306_WIDTH 128u

static const uint8_t font5x7[][5] PROGMEM = {
	{0x00, 0x00, 0x00, 0x00, 0x00}, /* space */
	{0x7E, 0x11, 0x11, 0x11, 0x7E}, /* A */
	{0x7F, 0x49, 0x49, 0x49, 0x36}, /* B */
	{0x3E, 0x41, 0x41, 0x41, 0x22}, /* C */
	{0x7F, 0x41, 0x41, 0x22, 0x1C}, /* D */
	{0x7F, 0x49, 0x49, 0x49, 0x41}, /* E */
	{0x7F, 0x09, 0x09, 0x09, 0x01}, /* F */
	{0x3E, 0x41, 0x49, 0x49, 0x7A}, /* G */
	{0x7F, 0x08, 0x08, 0x08, 0x7F}, /* H */
	{0x00, 0x41, 0x7F, 0x41, 0x00}, /* I */
	{0x20, 0x40, 0x41, 0x3F, 0x01}, /* J */
	{0x7F, 0x08, 0x14, 0x22, 0x41}, /* K */
	{0x7F, 0x40, 0x40, 0x40, 0x40}, /* L */
	{0x7F, 0x02, 0x0C, 0x02, 0x7F}, /* M */
	{0x7F, 0x04, 0x08, 0x10, 0x7F}, /* N */
	{0x3E, 0x41, 0x41, 0x41, 0x3E}, /* O */
	{0x7F, 0x09, 0x09, 0x09, 0x06}, /* P */
	{0x3E, 0x41, 0x51, 0x21, 0x5E}, /* Q */
	{0x7F, 0x09, 0x19, 0x29, 0x46}, /* R */
	{0x46, 0x49, 0x49, 0x49, 0x31}, /* S */
	{0x01, 0x01, 0x7F, 0x01, 0x01}, /* T */
	{0x3F, 0x40, 0x40, 0x40, 0x3F}, /* U */
	{0x1F, 0x20, 0x40, 0x20, 0x1F}, /* V */
	{0x3F, 0x40, 0x38, 0x40, 0x3F}, /* W */
	{0x63, 0x14, 0x08, 0x14, 0x63}, /* X */
	{0x07, 0x08, 0x70, 0x08, 0x07}, /* Y */
	{0x61, 0x51, 0x49, 0x45, 0x43}, /* Z */
	{0x3E, 0x51, 0x49, 0x45, 0x3E}, /* 0 */
	{0x00, 0x42, 0x7F, 0x40, 0x00}, /* 1 */
	{0x42, 0x61, 0x51, 0x49, 0x46}, /* 2 */
	{0x21, 0x41, 0x45, 0x4B, 0x31}, /* 3 */
	{0x18, 0x14, 0x12, 0x7F, 0x10}, /* 4 */
	{0x27, 0x45, 0x45, 0x45, 0x39}, /* 5 */
	{0x3C, 0x4A, 0x49, 0x49, 0x30}, /* 6 */
	{0x01, 0x71, 0x09, 0x05, 0x03}, /* 7 */
	{0x36, 0x49, 0x49, 0x49, 0x36}, /* 8 */
	{0x06, 0x49, 0x49, 0x29, 0x1E}, /* 9 */
	{0x00, 0x36, 0x36, 0x00, 0x00}, /* : */
	{0x08, 0x08, 0x08, 0x08, 0x08}, /* - */
	{0x00, 0x60, 0x60, 0x00, 0x00}  /* . */
};

static uint8_t line_buffer[SSD1306_WIDTH];
static bool display_ready;
static uint8_t current_screen;

static uint8_t glyph_index(char character)
{
	if (character == ' ') {
		return 0u;
	}
	if (character >= 'A' && character <= 'Z') {
		return (uint8_t)(character - 'A' + 1);
	}
	if (character >= '0' && character <= '9') {
		return (uint8_t)(character - '0' + 27);
	}
	if (character == ':') {
		return 37u;
	}
	if (character == '-') {
		return 38u;
	}
	if (character == '.') {
		return 39u;
	}
	return 0u;
}

bool oled_screen_clear(void)
{
	uint8_t page;
	for (page = 0u; page < SSD1306_PAGE_COUNT; ++page) {
		if (!oled_screen_write_line(page, "")) {
			return false;
		}
	}
	return true;
}

bool oled_screen_write_line(uint8_t page, const char *text)
{
	uint8_t commands[3];
	uint8_t column = 0u;
	uint8_t index;

	if (page >= SSD1306_PAGE_COUNT || text == 0) {
		return false;
	}

	for (index = 0u; index < SSD1306_WIDTH; ++index) {
		line_buffer[index] = 0u;
	}

	while (*text != '\0' && (uint16_t)column + 5u <= SSD1306_WIDTH) {
		uint8_t glyph = glyph_index(*text++);
		uint8_t glyph_column;

		for (glyph_column = 0u; glyph_column < 5u; ++glyph_column) {
			line_buffer[column++] = pgm_read_byte(&font5x7[glyph][glyph_column]);
		}
		++column;
	}

	commands[0] = (uint8_t)(0xB0u | page);
	commands[1] = 0x00u;
	commands[2] = 0x10u;
	if (!twi_write_packet(OLED_I2C_ADDRESS, 0x00u, commands, sizeof(commands))) {
		return false;
	}

	return twi_write_packet(OLED_I2C_ADDRESS, 0x40u, line_buffer, sizeof(line_buffer));
}

bool oled_screen_init(void)
{
	static const uint8_t commands[] = {
		0xAEu, 0xD5u, 0x80u, 0xA8u, 0x3Fu, 0xD3u, 0x00u, 0x40u,
		0x8Du, 0x14u, 0x20u, 0x02u, 0xA1u, 0xC8u, 0xDAu, 0x12u,
		0x81u, 0xCFu, 0xD9u, 0xF1u, 0xDBu, 0x40u, 0xA4u, 0xA6u,
		0xAFu
	};

	current_screen = 0u;
	if (!twi_write_packet(OLED_I2C_ADDRESS, 0x00u, commands, (uint8_t)sizeof(commands))) {
		display_ready = false;
		return false;
	}

	display_ready = oled_screen_clear();
	return display_ready;
}

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

void oled_screen_handle_encoder_event(encoder_event_t event)
{
	if (event == ENCODER_EVENT_CW) {
		if (current_screen < 2u) {
			current_screen++;
		}
	} else if (event == ENCODER_EVENT_CCW) {
		if (current_screen > 0u) {
			current_screen--;
		}
	} else if (event == ENCODER_EVENT_PRESS) {
		buzzer_beep(45u);
	}
}

bool oled_screen_render(bool sample_valid, const ina233_raw_sample_t *sample)
{
	char line[22];
	bool ok = true;

	if (!display_ready) {
		return false;
	}

	if (current_screen == 0u) {
		ok = oled_screen_write_line(0u, "SMART CHARGER") && ok;
		ok = oled_screen_write_line(1u, "MONITOR ONLY") && ok;
		ok = oled_screen_write_line(2u, "CHG ALL OFF") && ok;
		ok = oled_screen_write_line(3u, "DSG ALL OFF") && ok;
		ok = oled_screen_write_line(4u, sample_valid ? "INA233: OK" : "INA233: ERROR") && ok;
		ok = oled_screen_write_line(5u, "TURN FOR DATA") && ok;
	} else if (current_screen == 1u) {
		ok = oled_screen_write_line(0u, "INA233 DATA") && ok;
		if (sample_valid && sample != 0) {
			format_float_value(line, "VBUS: ", sample->bus_voltage_v, " V");
			ok = oled_screen_write_line(1u, line) && ok;
			format_float_value(line, "VSHU: ", sample->shunt_voltage_mv, " mV");
			ok = oled_screen_write_line(2u, line) && ok;
			format_float_value(line, "CURR: ", sample->current_ma, " mA");
			ok = oled_screen_write_line(3u, line) && ok;
			format_float_value(line, "POW:  ", sample->power_mw, " mW");
			ok = oled_screen_write_line(4u, line) && ok;
			ok = oled_screen_write_line(5u, "PAGE 2: RAW") && ok;
		} else {
			ok = oled_screen_write_line(1u, "SENSOR ERROR") && ok;
			ok = oled_screen_write_line(2u, "") && ok;
			ok = oled_screen_write_line(3u, "") && ok;
			ok = oled_screen_write_line(4u, "") && ok;
			ok = oled_screen_write_line(5u, "") && ok;
		}
	} else {
		ok = oled_screen_write_line(0u, "INA233 RAW") && ok;
		if (sample_valid && sample != 0) {
			format_raw_value(line, "VBUS:", sample->bus_voltage_raw);
			ok = oled_screen_write_line(1u, line) && ok;
			format_raw_value(line, "VSHU:", sample->shunt_voltage_raw);
			ok = oled_screen_write_line(2u, line) && ok;
			format_raw_value(line, "CURR:", sample->current_raw);
			ok = oled_screen_write_line(3u, line) && ok;
			format_raw_value(line, "POW:", sample->power_raw);
			ok = oled_screen_write_line(4u, line) && ok;
			format_raw_value(line, "ENGY:", sample->energy_raw);
			ok = oled_screen_write_line(5u, line) && ok;
		} else {
			ok = oled_screen_write_line(1u, "SENSOR ERROR") && ok;
			ok = oled_screen_write_line(2u, "") && ok;
			ok = oled_screen_write_line(3u, "") && ok;
			ok = oled_screen_write_line(4u, "") && ok;
			ok = oled_screen_write_line(5u, "") && ok;
		}
	}

	if (!ok) {
		display_ready = false;
	}
	return ok;
}
