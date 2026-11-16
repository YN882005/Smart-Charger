#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

typedef enum {
	ENCODER_EVENT_NONE = 0,
	ENCODER_EVENT_CW,
	ENCODER_EVENT_CCW,
	ENCODER_EVENT_PRESS,
	ENCODER_EVENT_DOUBLE_PRESS
} encoder_event_t;

void encoder_init(void);
encoder_event_t encoder_poll(void);

#endif
