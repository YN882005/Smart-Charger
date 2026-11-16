#include "OLED_Screen.h"

#include <avr/pgmspace.h>
#include <stdint.h>

#include "buzzer.h"
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
	{0x00, 0x60, 0x60, 0x00, 0x00}, /* . */
	{0x00, 0x41, 0x22, 0x14, 0x08}, /* > */
	{0x00, 0x1C, 0x22, 0x41, 0x00}, /* ( */
	{0x00, 0x41, 0x22, 0x1C, 0x00}  /* ) */
};

static uint8_t line_buffer[SSD1306_WIDTH];
static bool display_ready;
typedef enum {
	UI_SCREEN_MAIN = 0,
	UI_SCREEN_CHANNEL,
	UI_SCREEN_MODE,
	UI_SCREEN_CURRENT_LIMIT,
	UI_SCREEN_LOCKED
} ui_screen_t;

typedef enum {
	UI_MODE_CHARGE = 0,
	UI_MODE_DISCHARGE
} ui_mode_t;

static ui_screen_t current_screen;
static uint8_t selected_channel;
static ui_mode_t selected_mode[INA233_CHANNEL_COUNT] = {
	UI_MODE_CHARGE,
	UI_MODE_CHARGE
};
static uint16_t selected_current_limit[INA233_CHANNEL_COUNT] = {
	CHG_CURRENT_DEFAULT_MA,
	CHG_CURRENT_DEFAULT_MA
};

static uint8_t glyph_index(char character)
{
	if (character >= 'a' && character <= 'z') {
		character = (char)(character - 'a' + 'A');
	}
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
	if (character == '>') {
		return 40u;
	}
	if (character == '(') {
		return 41u;
	}
	if (character == ')') {
		return 42u;
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

	current_screen = UI_SCREEN_MAIN;
	selected_channel = 0u;
	selected_mode[0] = UI_MODE_CHARGE;
	selected_mode[1] = UI_MODE_CHARGE;
	selected_current_limit[0] = CHG_CURRENT_DEFAULT_MA;
	selected_current_limit[1] = CHG_CURRENT_DEFAULT_MA;
	if (!twi_write_packet(OLED_I2C_ADDRESS, 0x00u, commands, (uint8_t)sizeof(commands))) {
		display_ready = false;
		return false;
	}

	display_ready = oled_screen_clear();
	return display_ready;
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

static void set_screen(ui_screen_t screen)
{
	if (current_screen != screen) {
		current_screen = screen;
		buzzer_beep(BUZZER_BEEP_MS);
	}
}

static bool channel_is_present(bool sample_valid, const ina233_raw_sample_t *sample)
{
	uint32_t voltage_mv;

	if (!sample_valid || sample == 0) {
		return false;
	}

	voltage_mv = ((uint32_t)sample->bus_voltage_raw * 5u) / 4u;
	return voltage_mv >= BATT_PRESENCE_THRESHOLD_MV;
}

static const char *channel_status(uint8_t channel, bool sample_valid,
	const ina233_raw_sample_t *sample)
{
	if (!sample_valid || sample == 0) {
		return channel == 0u ? "CH1 SENSOR ERROR" : "CH2 SENSOR ERROR";
	}
	if (!channel_is_present(sample_valid, sample)) {
		return channel == 0u ? "CH1 NO CELL :-(" : "CH2 NO CELL :-(";
	}
	return channel == 0u ? "CH1 SOC: N/A" : "CH2 SOC: N/A";
}

static bool render_main_screen(const bool sample_valid[INA233_CHANNEL_COUNT],
	const ina233_raw_sample_t samples[INA233_CHANNEL_COUNT], bool alert_active)
{
	char line[22];
	bool ok = true;
	uint8_t channel;

	ok = oled_screen_write_line(0u, "DUAL MONITOR ONLY") && ok;
	for (channel = 0u; channel < (uint8_t)INA233_CHANNEL_COUNT; ++channel) {
		uint8_t voltage_page = (uint8_t)(1u + channel * 3u);
		uint8_t current_page = (uint8_t)(voltage_page + 1u);
		uint8_t status_page = (uint8_t)(voltage_page + 2u);
		const char *voltage_label = channel == 0u ? "CH1: " : "CH2: ";
		const char *current_label = channel == 0u ? "CH1 I: " : "CH2 I: ";

		if (sample_valid[channel]) {
			format_float_value(line, voltage_label, samples[channel].bus_voltage_v, " V");
			ok = oled_screen_write_line(voltage_page, line) && ok;
			format_float_value(line, current_label, samples[channel].current_ma, " mA");
			ok = oled_screen_write_line(current_page, line) && ok;
		} else {
			ok = oled_screen_write_line(voltage_page, channel == 0u
				? "CH1 SENSOR ERROR" : "CH2 SENSOR ERROR") && ok;
			ok = oled_screen_write_line(current_page, "") && ok;
		}

		ok = oled_screen_write_line(status_page,
			channel_status(channel, sample_valid[channel], &samples[channel])) && ok;
	}

	ok = oled_screen_write_line(7u,
		alert_active ? "INA ALERT: ACTIVE" : "INA ALERT: CLEAR") && ok;
	return ok;
}

static bool render_menu_screen(void)
{
	char line[22];
	bool ok = true;

	if (current_screen == UI_SCREEN_CHANNEL) {
		ok = oled_screen_write_line(0u, "SELECT CHANNEL") && ok;
		ok = oled_screen_write_line(1u, selected_channel == 0u
			? "> CH1: LION" : "  CH1: LION") && ok;
		ok = oled_screen_write_line(2u, selected_channel == 1u
			? "> CH2: LIFEPO4" : "  CH2: LIFEPO4") && ok;
		ok = oled_screen_write_line(3u, "ROTATE TO SELECT") && ok;
		ok = oled_screen_write_line(4u, "PRESS TO CONTINUE") && ok;
		ok = oled_screen_write_line(5u, "DOUBLE PRESS HOME") && ok;
		ok = oled_screen_write_line(6u, "MONITOR ONLY") && ok;
		ok = oled_screen_write_line(7u, "OUTPUTS LOCKED") && ok;
		return ok;
	}

	if (current_screen == UI_SCREEN_MODE) {
		ok = oled_screen_write_line(0u, selected_channel == 0u
			? "CH1 LION CONFIG" : "CH2 LIFEPO4 CONFIG") && ok;
		ok = oled_screen_write_line(1u, selected_mode[selected_channel] == UI_MODE_CHARGE
			? "> MODE: CHARGE" : "> MODE: DISCHARGE") && ok;
		ok = oled_screen_write_line(2u, "ROTATE TO CHANGE") && ok;
		ok = oled_screen_write_line(3u, "PRESS TO CONTINUE") && ok;
		format_float_value(line, "LIMIT: ",
			(float)selected_current_limit[selected_channel], "mA");
		ok = oled_screen_write_line(4u, line) && ok;
		ok = oled_screen_write_line(5u, "100 TO 1200MA") && ok;
		ok = oled_screen_write_line(6u, "CONTROL LOCKED") && ok;
		ok = oled_screen_write_line(7u, "DOUBLE PRESS HOME") && ok;
		return ok;
	}

	if (current_screen == UI_SCREEN_CURRENT_LIMIT) {
		ok = oled_screen_write_line(0u, "CHARGE CURRENT") && ok;
		format_float_value(line, "LIMIT: ",
			(float)selected_current_limit[selected_channel], "mA");
		ok = oled_screen_write_line(1u, line) && ok;
		ok = oled_screen_write_line(2u, "ROTATE 100-1200MA") && ok;
		ok = oled_screen_write_line(3u, "PRESS TO CONFIRM") && ok;
		ok = oled_screen_write_line(4u, "CHARGING LOCKED") && ok;
		ok = oled_screen_write_line(5u, "POWER OUTPUT OFF") && ok;
		ok = oled_screen_write_line(6u, "DOUBLE PRESS HOME") && ok;
		ok = oled_screen_write_line(7u, "") && ok;
		return ok;
	}

	ok = oled_screen_write_line(0u, "CONTROL LOCKED") && ok;
	ok = oled_screen_write_line(1u, "NO POWER CONTROL") && ok;
	ok = oled_screen_write_line(2u, "VALIDATION NEEDED") && ok;
	ok = oled_screen_write_line(3u, "CHARGE AND DSG OFF") && ok;
	ok = oled_screen_write_line(4u, "PRESS FOR HOME") && ok;
	ok = oled_screen_write_line(5u, "DOUBLE PRESS HOME") && ok;
	ok = oled_screen_write_line(6u, "") && ok;
	ok = oled_screen_write_line(7u, "") && ok;
	return ok;
}

void oled_screen_handle_encoder_event(encoder_event_t event)
{
	if (event == ENCODER_EVENT_DOUBLE_PRESS) {
		set_screen(UI_SCREEN_MAIN);
		return;
	}

	if (event == ENCODER_EVENT_CW || event == ENCODER_EVENT_CCW) {
		if (current_screen == UI_SCREEN_MAIN) {
			set_screen(UI_SCREEN_CHANNEL);
		} else if (current_screen == UI_SCREEN_CHANNEL) {
			selected_channel ^= 1u;
		} else if (current_screen == UI_SCREEN_MODE) {
			selected_mode[selected_channel] = selected_mode[selected_channel]
				== UI_MODE_CHARGE ? UI_MODE_DISCHARGE : UI_MODE_CHARGE;
		} else if (current_screen == UI_SCREEN_CURRENT_LIMIT) {
			if (event == ENCODER_EVENT_CW
				&& selected_current_limit[selected_channel] < CHG_CURRENT_MAX_MA) {
				selected_current_limit[selected_channel] += CHG_CURRENT_STEP_MA;
			} else if (event == ENCODER_EVENT_CCW
				&& selected_current_limit[selected_channel] > CHG_CURRENT_MIN_MA) {
				selected_current_limit[selected_channel] -= CHG_CURRENT_STEP_MA;
			}
		}
		return;
	}

	if (event != ENCODER_EVENT_PRESS) {
		return;
	}

	if (current_screen == UI_SCREEN_CHANNEL) {
		set_screen(UI_SCREEN_MODE);
	} else if (current_screen == UI_SCREEN_MODE) {
		set_screen(selected_mode[selected_channel] == UI_MODE_CHARGE
			? UI_SCREEN_CURRENT_LIMIT : UI_SCREEN_LOCKED);
	} else if (current_screen == UI_SCREEN_CURRENT_LIMIT) {
		set_screen(UI_SCREEN_LOCKED);
	} else if (current_screen == UI_SCREEN_LOCKED) {
		set_screen(UI_SCREEN_MAIN);
	}
}

bool oled_screen_render(const bool sample_valid[INA233_CHANNEL_COUNT],
	const ina233_raw_sample_t samples[INA233_CHANNEL_COUNT], bool alert_active)
{
	bool ok;

	if (!display_ready || sample_valid == 0 || samples == 0) {
		return false;
	}

	if (current_screen == UI_SCREEN_MAIN) {
		ok = render_main_screen(sample_valid, samples, alert_active);
	} else {
		ok = render_menu_screen();
	}

	if (!ok) {
		display_ready = false;
	}
	return ok;
}
