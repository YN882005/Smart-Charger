#ifndef ENCODER_H
#define ENCODER_H

typedef enum {
	ENCODER_EVENT_NONE = 0,
	ENCODER_EVENT_CW,
	ENCODER_EVENT_CCW,
	ENCODER_EVENT_PRESS
} encoder_event_t;

void encoder_init(void);
encoder_event_t encoder_poll(void);

#endif
