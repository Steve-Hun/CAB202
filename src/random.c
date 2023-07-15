#include <stdint.h>
#include "globals.h"

volatile uint32_t new_state_lfsr = 0x11395508;
volatile uint32_t start_state_lfsr = 0x11395508;
volatile uint32_t state_lfsr = 0x11395508;
volatile uint8_t next_lfsr_digit = 0; // stores the next digit from LFSR
volatile uint8_t set_new_seed = 0;

// Reset Linear-feedback shift register's seed to new seed (if any) or reset back to default seed
void reset_lfsr_state(void)
{
    start_state_lfsr = new_state_lfsr;
}

// Linear shift by one, update new_lfsr_state to new digit
void next(void)
{
    uint16_t shifted_bit = state_lfsr & 0b1;
    state_lfsr = state_lfsr >> 1;
    if (shifted_bit == 1)
    {
        state_lfsr = state_lfsr ^ 0xE2023CAB; // Mask
    }
    next_lfsr_digit = state_lfsr & 0b11;
}