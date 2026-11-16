#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

void board_init_safe(void);
void board_charge_disable(void);
void board_discharge_disable(void);
void board_outputs_safe(void);
bool board_outputs_are_safe(void);

#endif
