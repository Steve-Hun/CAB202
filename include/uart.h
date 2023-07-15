#ifndef UART_H
#define UART_H 1

#include <stdint.h>

extern volatile uint8_t reset;
extern volatile Serial_State SERIAL_STATE;

uint8_t uart_getc(void);
void uart_putc(uint8_t c);
void uart_puts(char *string);
void reset_game(void);

#endif
