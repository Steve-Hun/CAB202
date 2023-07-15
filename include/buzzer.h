// Ensure that this file is only included once
#ifndef BUZZER_H
#define BUZZER_H 1

#include <stdint.h>

extern volatile int8_t octave; // default 0

void play_tone(uint8_t sequence_digit);
void reset_frequency(void);
void stop_tone(void);

#endif
