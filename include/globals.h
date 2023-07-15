// Prevent multiple inclusion of the header
#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>
#include "types.h"

// External variables for main program

extern volatile State STATE;
extern volatile High_Score_State HIGHSCORE_STATE;
extern volatile User_Turn_State USER_STATE;
extern volatile Simon_Turn_State SIMON_STATE;
extern volatile Result_State RESULT_STATE;
extern volatile int32_t tone_index; // helps keep track of current tone playing in a sequence
extern volatile uint8_t pb_released;
extern volatile uint16_t sequence_length;
extern volatile uint32_t sequence_matched;
extern volatile uint8_t user_input_index;
extern volatile uint8_t save_entry;
extern volatile char name_entry[20];
extern volatile uint8_t current_index;

#endif
