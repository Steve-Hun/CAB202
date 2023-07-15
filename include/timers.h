#ifndef TIMERS_H
#define TIMERS_H 1

#include <stdint.h>

extern volatile uint16_t elapsed_time;
extern volatile uint16_t playback_time;
extern volatile uint16_t new_playback_time;
extern volatile uint8_t allow_updating_playback_delay;

extern volatile uint8_t pb_state;

#endif
