#include <stdint.h>

#include <avr/io.h>
#include "spi.h"
#include "globals.h"

// ------------------------  BUZZER  ------------------------

// Period for lowest frequency (octave: -2)
#define PERIOD_E_HIGH 43572
#define PERIOD_E_LOW 87144
#define PERIOD_A 32680
#define PERIOD_C_SHARP 51880

static uint32_t periods[4] = {PERIOD_E_HIGH, PERIOD_C_SHARP, PERIOD_A, PERIOD_E_LOW};

volatile int8_t octave = 0;

// Reset frequency to default frequency
void reset_frequency(void)
{
    octave = 0;
};

// Play a tone frequency based on sequence digit
void play_tone(uint8_t sequence_digit)
{
    TCA0.SINGLE.PERBUF = periods[sequence_digit] >> (octave + 2);
    TCA0.SINGLE.CMP0BUF = TCA0.SINGLE.PERBUF >> 1;

    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

// Stop buzzer from playing sound
void stop_tone(void)
{
    TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
}
