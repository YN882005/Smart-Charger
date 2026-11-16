#ifndef SSD1306_H
#define SSD1306_H

#include <stdbool.h>
#include <stdint.h>

bool ssd1306_init(void);
bool ssd1306_write_line(uint8_t page, const char *text);

#endif
