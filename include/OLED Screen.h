#ifndef OLED_SCREEN_H
#define OLED_SCREEN_H

#include <stdbool.h>
#include <stdint.h>

#include "Encoder.h"
#include "ina233.h"

/* Low-level display driver functions */
bool oled_screen_init(void);
bool oled_screen_clear(void);
bool oled_screen_write_line(uint8_t page, const char *text);

/* Screen rendering and user-interaction functions */
void oled_screen_handle_encoder_event(encoder_event_t event);
bool oled_screen_render(bool sample_valid, const ina233_raw_sample_t *sample);

/* Backwards-compatibility aliases */
#define ssd1306_init oled_screen_init
#define ssd1306_clear oled_screen_clear
#define ssd1306_write_line oled_screen_write_line
#define ui_init oled_screen_init
#define ui_handle_encoder_event oled_screen_handle_encoder_event
#define ui_render oled_screen_render

#endif
