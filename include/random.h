#ifndef RANDOM_H
#define RANDOM_H 1

#include <stdint.h>

extern volatile uint32_t new_state_lfsr;
extern volatile uint32_t start_state_lfsr;
extern volatile uint32_t state_lfsr;
extern volatile uint8_t next_lfsr_digit;
extern volatile uint8_t set_new_seed;

void next();
void reset_lfsr_state();
void update_seed(uint32_t seed);

#endif
