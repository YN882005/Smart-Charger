#ifndef TWI_H
#define TWI_H

#include <stdbool.h>
#include <stdint.h>

void twi_init(void);
bool twi_read_register_word(uint8_t address, uint8_t command, uint16_t *value);

#endif
