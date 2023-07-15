#ifndef SPI_H
#define SPI_H 1

#include <avr/interrupt.h>
#include <avr/io.h>

extern volatile uint8_t number_segs[10];
extern uint8_t segs[2];

void spi_init(void);
void spi_write(uint8_t b);
void display_tone(uint8_t sequence_digit);
void clear_display(void);
void display_success(void);
void display_fail(void);
void display_score(uint32_t score);

#endif
