#ifndef BUZZER_H
#define BUZZER_H

#include <stdbool.h>
#include <stdint.h>

void buzzer_init(void);
void buzzer_beep(uint16_t duration_ms);
bool buzzer_is_beeping(void);

#endif /* BUZZER_H */
