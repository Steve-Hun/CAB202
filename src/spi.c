#include <avr/interrupt.h>
#include <avr/io.h>

#include "types.h"
#include "globals.h"

//    xFABGCDE
// a: 10111110
// b: 11101011
// c: 00111110
// d: 01101011
// 0: 00001000
// 1: 01101011
// 2: 01000100
// 3: 01000001
// 4: 00100011
// 5: 00010001
// 6: 00010000
// 7: 01001011
// 8: 00000000
// 9: 00000001

//     ABCDEFG
//    xFABGCDE
#define SEGS_EF 0b00111110
#define SEGS_BC 0b01101011
#define SEGS_OFF 0b01111111
#define SEGS_SUCCESS 0b00000000
#define SEGS_FAIL 0b01110111

volatile uint8_t segs[2] = {SEGS_OFF, SEGS_OFF}; // segs initially off

volatile uint8_t number_segs[10] = {
    0x08, 0x6B, 0x44, 0x41, 0x23, 0x11, 0x10, 0x4B, 0x00, 0x01};

void spi_write(uint8_t b)
{
    SPI0.DATA = b; // Note DATA register used for both Tx and Rx
}

void spi_init(void)
{
    cli();
    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc; // SPI pins on PC0-3
    PORTC.DIRSET = (PIN0_bm | PIN2_bm);       // SCK (PC0) and MOSI (PC2) output
    PORTA.OUTSET = PIN1_bm;                   // DISP_LATCH initiall high
    PORTA.DIRSET = PIN1_bm;                   // Output

    SPI0.CTRLA = SPI_MASTER_bm;  // Master, /4 prescaler, MSB first
    SPI0.CTRLB = SPI_SSD_bm;     // Mode 0, client select disable, unbuffered
    SPI0.INTCTRL = SPI_IE_bm;    // Interrupt enable
    SPI0.CTRLA |= SPI_ENABLE_bm; // Enable
    sei();
}

// Displaying tone on 7-segmens display based on current sequence digit
void display_tone(uint8_t sequence_digit)
{
    switch (sequence_digit)
    {
    case TONE_1:
        segs[0] = SEGS_EF;
        segs[1] = SEGS_OFF;
        break;
    case TONE_2:
        segs[0] = SEGS_BC;
        segs[1] = SEGS_OFF;
        break;
    case TONE_3:
        segs[0] = SEGS_OFF;
        segs[1] = SEGS_EF;
        break;
    case TONE_4:
        segs[0] = SEGS_OFF;
        segs[1] = SEGS_BC;
        break;
    default:
        break;
    }
}

void clear_display(void)
{
    segs[0] = SEGS_OFF;
    segs[1] = SEGS_OFF;
}

void display_success(void)
{
    segs[0] = SEGS_SUCCESS;
    segs[1] = SEGS_SUCCESS;
}

void display_fail(void)
{
    segs[0] = SEGS_FAIL;
    segs[1] = SEGS_FAIL;
}

void display_score(uint32_t count)
{
    uint32_t num, tens = 0;

    num = count;
    if (num < 10)
    {
        segs[0] = SEGS_OFF;
        segs[1] = number_segs[num];
        return;
    }

    while (num > 9)
    {
        num -= 10;
        tens++;
    }

    segs[0] = number_segs[tens];
    segs[1] = number_segs[num];
}

ISR(SPI0_INT_vect)
{
    // rising edge on DISP_LATCH
    PORTA.OUTCLR = PIN1_bm;
    PORTA.OUTSET = PIN1_bm;
    SPI0.INTFLAGS = SPI_IF_bm; // reset interrupt flag
}