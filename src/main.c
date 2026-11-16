#include <stdbool.h>
#include <stdint.h>
#include <util/delay.h>

#include "board.h"
#include "config.h"
#include "ina233.h"
#include "twi.h"

volatile ina233_raw_sample_t g_ina233_sample;
volatile bool g_ina233_sample_valid;

int main(void)
{
	board_init_safe();
	twi_init();

	for (;;) {
		ina233_raw_sample_t sample;

		board_outputs_safe();
		g_ina233_sample_valid = ina233_read_sample(&sample);
		if (g_ina233_sample_valid) {
			g_ina233_sample = sample;
		}

		_delay_ms(100);
	}
}
