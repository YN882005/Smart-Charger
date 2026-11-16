#ifndef CHARGING_H
#define CHARGING_H

#include <stdbool.h>

void charging_init_safe(void);
void charging_disable(void);
bool charging_outputs_are_safe(void);

#endif
