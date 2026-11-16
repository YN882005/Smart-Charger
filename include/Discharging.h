#ifndef DISCHARGING_H
#define DISCHARGING_H

#include <stdbool.h>

void discharging_init_safe(void);
void discharging_disable(void);
bool discharging_outputs_are_safe(void);

#endif
