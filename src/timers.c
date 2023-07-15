#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include "spi.h"
#include "globals.h"

// ------------------------  TIMING  ------------------------
extern uint8_t segs[];

volatile uint16_t elapsed_time = 0;
volatile uint16_t playback_time = 500;
volatile uint16_t new_playback_time = 2000;

volatile uint8_t allow_updating_playback_delay = 1;
volatile uint8_t pb_state = 0xFF;

void pb_debounce(void)
{
    static uint8_t count0 = 0;
    static uint8_t count1 = 0;

    uint8_t pb_edge = pb_state ^ PORTA.IN;

    // Vertical counter
    count1 = (count1 ^ count0) & pb_edge;
    count0 = ~count0 & pb_edge;

    // New debounced state
    // Update if PB high for three samples
    pb_state ^= (count1 & count0);
}

ISR(TCB0_INT_vect)
{
    elapsed_time++; // each ms passed, it increases

    if (allow_updating_playback_delay)
    {
        playback_time = 250 + ((1757UL * ADC0.RESULT) >> 8);
    }

    TCB0.INTFLAGS = TCB_CAPT_bm; // reset interrupt flag
}

// ------------------------  DEBOUNCED PUSH BUTTON HANDLING  ------------------------

ISR(TCB1_INT_vect)
{
    pb_debounce();

    static uint8_t digit = 0;

    if (digit)
    {
        spi_write(segs[0] | (0x01 << 7));
    }
    else
    {
        spi_write(segs[1]);
    }
    digit = !digit;

    TCB1.INTFLAGS = TCB_CAPT_bm; // reset interrupt flag
}
