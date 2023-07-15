#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "buzzer.h"
#include "timers.h"
#include "types.h"
#include "random.h"
#include "spi.h"
#include "globals.h"

volatile uint8_t reset = 0;
volatile Serial_State SERIAL_STATE = AWAITING_COMMAND;

// ------------------------  SERIAL PARSER  ------------------------

uint8_t hexchar_to_int(char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    else
        return 16; // Invalid
}

uint8_t uart_getc(void)
{
    while (!(USART0.STATUS & USART_RXCIF_bm))
        ; // Wait for data
    return USART0.RXDATAL;
}

void uart_putc(uint8_t c)
{
    while (!(USART0.STATUS & USART_DREIF_bm))
        ; // Wait for TXDATA empty
    USART0.TXDATAL = c;
}

void uart_puts(char *string)
{
    while (*string != '\0')
    {
        uart_putc(*string);
        string++;
    }
}

// increase frequency of tones - INC FREQ
void increment_frequency(void)
{
    if (octave < 2)
    {
        octave++;
    }
}

// decrease frequency of tones - DEC FREQ
void decrement_frequency(void)
{
    if (octave > -2)
    {
        octave--;
    }
}

// reset frequencies to default, reset sequence, etc. - RESET
void reset_game(void)
{
    reset_frequency();
    reset_lfsr_state();
    stop_tone();
    clear_display();
    reset = 1;
}

ISR(USART0_RXC_vect)
{
    static uint8_t chars_received = 0;
    static uint32_t payload = 0;
    static uint8_t payload_valid = 1;

    uint8_t rx_data = USART0.RXDATAL; // read data from serial

    switch (SERIAL_STATE)
    {
    case AWAITING_COMMAND:
        switch (rx_data)
        {
        case '1':
        case '2':
        case '3':
        case '4':
        case 'q':
        case 'w':
        case 'e':
        case 'r':
            if (STATE == USER_TURN && USER_STATE == PAUSED)
            {
                uart_putc(rx_data);
                uart_putc('\n');
                if (rx_data == '1' || rx_data == 'q') // Same as Button 1 pressed
                {
                    play_tone(TONE_1);
                    display_tone(TONE_1);
                    pb_released = 1;
                    elapsed_time = 0;
                    allow_updating_playback_delay = 0; // not allow update playback delay while sound is playing
                    user_input_index = 0;
                    USER_STATE = PLAYING;
                }
                else if (rx_data == '2' || rx_data == 'w') // Same as Button 2 pressed
                {
                    play_tone(TONE_2);
                    display_tone(TONE_2);
                    pb_released = 1;
                    elapsed_time = 0;
                    allow_updating_playback_delay = 0; // not allow update playback delay while sound is playing
                    user_input_index = 1;
                    USER_STATE = PLAYING;
                }
                else if (rx_data == '3' || rx_data == 'e') // Same as Button 3 pressed
                {
                    play_tone(TONE_3);
                    display_tone(TONE_3);
                    pb_released = 1;
                    elapsed_time = 0;
                    allow_updating_playback_delay = 0; // not allow update playback delay while sound is playing
                    user_input_index = 2;
                    USER_STATE = PLAYING;
                }
                else if (rx_data == '4' || rx_data == 'r') // Same as Button 4 pressed
                {
                    play_tone(TONE_4);
                    display_tone(TONE_4);
                    pb_released = 1;
                    elapsed_time = 0;
                    allow_updating_playback_delay = 0; // not allow update playback delay while sound is playing
                    user_input_index = 3;
                    USER_STATE = PLAYING;
                }
            }
            break;
        case ',':
        case 'k':
            increment_frequency();
            break;
        case '.':
        case 'l':
            decrement_frequency();
            break;
        case '0':
        case 'p':
            reset_game(); // reset frequencies to default, reset sequence, etc.
            break;
        case '9':
        case 'o':
            uart_putc(rx_data);
            payload_valid = 1;
            chars_received = 0;
            payload = 0;
            SERIAL_STATE = AWAITING_PAYLOAD;
            break;
        default:
            break;
        }
        break;
    case AWAITING_PAYLOAD:
    {
        uart_putc(rx_data);
        uint8_t parsed_result = hexchar_to_int((char)rx_data);
        if (parsed_result != 16)
            payload = (payload << 4) | parsed_result;
        else
        {
            payload_valid = 0;
        }

        if (++chars_received == 8)
        {
            if (payload_valid)
            {
                new_state_lfsr = payload;
                set_new_seed = 1;
            }
            SERIAL_STATE = AWAITING_COMMAND; // Regardless of payload validity, after 8 characters, the state is changed back
        }
        break;
    }
    case AWAITING_NAME:
    {

        static uint8_t characters_received = 0;
        uart_putc(rx_data);

        if (rx_data == '\r') // ENTER pressed
        {
            save_entry = 1;
            SERIAL_STATE = AWAITING_NEWLINE; // On Window, ENTER key is "\r\n"
        }
        else if (rx_data == '\n')
        {
            save_entry = 1;
            SERIAL_STATE = AWAITING_COMMAND;
        }
        else
        {
            name_entry[current_index] = rx_data;
            current_index++;
            elapsed_time = 0;
        }

        if (++characters_received == 20)
        {
            save_entry = 1;
        }
        break;
    }

    case AWAITING_NEWLINE:
    {
        // Ignore the newline character and return to AWAITING_NAME state
        SERIAL_STATE = AWAITING_NAME;
        break;
    }

    default:
        break;
    }
}
