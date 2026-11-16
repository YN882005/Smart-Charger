#ifndef UI_H
#define UI_H

#include <stdbool.h>

#include "encoder.h"
#include "ina233.h"

bool ui_init(void);
void ui_handle_encoder_event(encoder_event_t event);
bool ui_render(bool sample_valid, const ina233_raw_sample_t *sample);

#endif
