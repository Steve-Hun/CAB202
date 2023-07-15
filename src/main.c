#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "buzzer.h"
#include "initialisation.h"
#include "timers.h"
#include "types.h"
#include "random.h"
#include "spi.h"
#include "uart.h"
#include "globals.h" // stores external variables for main

// Requirements for printf functions
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
int uart_putc_printf(char c, FILE *stream)
{
    uart_putc(c);
    return 0;
}
static FILE mystdout = FDEV_SETUP_STREAM(uart_putc_printf, NULL, _FDEV_SETUP_WRITE);

const uint8_t MAX_HIGH_SCORE = 5;

volatile State STATE = INIT;
volatile High_Score_State HIGHSCORE_STATE = CHECK_SCORE;
volatile User_Turn_State USER_STATE = PAUSED;
volatile Simon_Turn_State SIMON_STATE = SILENT;
volatile Result_State RESULT_STATE = START;
volatile int32_t tone_index = 0; // helps keep track of current tone playing in a sequence
volatile uint8_t pb_released = 0;
volatile uint16_t sequence_length = 1;
volatile uint32_t sequence_matched = 1;
volatile uint8_t user_input_index = 0;
volatile uint8_t save_entry = 0;
volatile char name_entry[20];
volatile uint8_t current_index = 0;

score_entry high_score[5] = {0};
uint32_t user_score = 0;

// Check whether the sequence still matches
void is_sequence_matched(uint8_t tone_digit, uint8_t lfsr_digit)
{
    if (sequence_matched)
    {
        sequence_matched = tone_digit == lfsr_digit;
    }
}

// Check whether given score is in top five
uint8_t in_top_five(uint16_t score)
{
    for (int8_t index = 0; index < MAX_HIGH_SCORE; index++)
    {
        if (score > high_score[index].score)
        {
            return 1;
        }
    }
    return 0;
}

// Add name and score to the high score list
void add_high_score(char *name, uint16_t score)
{
    uint8_t index;
    // Find index location of currrent score in the top scorer array
    for (index = 0; index < MAX_HIGH_SCORE; index++)
    {
        if (score > high_score[index].score)
        {
            break;
        }
    }

    // Shift scores that are lower than given score
    for (int8_t j = MAX_HIGH_SCORE - 1; j > index; j--)
    {
        high_score[j] = high_score[j - 1];
    }

    strncpy(high_score[index].name, name, current_index);
    high_score[index].name[current_index] = '\0';
    high_score[index].score = score;
}

// Display high score on 7-segmet display
void display_high_scores(void)
{
    printf("\n");
    for (uint8_t i = 0; i < 5; ++i)
    {
        if (high_score[i].score != 0)
        {
            printf("%s", high_score[i].name);
            printf(" %d", high_score[i].score);
            printf("\n");
        }
    }
}

int main()
{
    cli();
    adc_init();
    buttons_init();
    port_init();
    pwm_init();
    timers_init();
    uart_init();
    spi_init();

    // Requirement for printf function
    stdout = &mystdout;

    sei();

    // Push button states
    uint8_t pb_previous_state = 0xFF;
    uint8_t pb_new_state = 0xFF;
    uint8_t pb_falling_edge, pb_rising_edge;
    uint16_t half_playback_time = 0;
    playback_time = 250 + ((1757UL * ADC0.RESULT) >> 8);

    while (1)
    {
        // Game play state machine encapsulates all possible state of the game
        switch (STATE)
        {
        case INIT:
            sequence_length = 1;
            allow_updating_playback_delay = 1;
            STATE = SIMON_TURN;
            break;

        case SIMON_TURN:
            if (set_new_seed)
            {
                reset_lfsr_state();
                set_new_seed = 0;
            }

            state_lfsr = start_state_lfsr; // reset to initial state
            _delay_ms(1);                  // delay 1ms to allow for updating new playback time
            tone_index = 0;
            SIMON_STATE = START;

            // Using while loop helps encapsulate the logic of each player turn
            while (1)
            {
                half_playback_time = playback_time >> 1;

                // Stop Simon's turn upon RESET or reach the end of Simon's sequence
                if (reset)
                    break;
                if (tone_index >= sequence_length)
                    break;

                // This state machine encapsulates the different states
                // that happen during Simon's turn
                switch (SIMON_STATE)
                {
                case START:
                    // Starting playing simon's sequence
                    next();
                    elapsed_time = 0;
                    allow_updating_playback_delay = 0; // disable playback update during play
                    play_tone(next_lfsr_digit);
                    display_tone(next_lfsr_digit);

                    SIMON_STATE = PLAY;
                    break;
                case PLAY:
                    if (elapsed_time >= half_playback_time)
                    {
                        stop_tone();
                        allow_updating_playback_delay = 1;
                        clear_display();
                        SIMON_STATE = SILENT;
                    }
                    break;
                case SILENT:
                    if (elapsed_time >= playback_time)
                    {
                        tone_index++;
                        SIMON_STATE = START;
                    }
                    break;
                default:
                    break;
                }
            }

            if (reset)
            {
                STATE = INIT;
                reset = 0;
                break;
            }

            STATE = USER_TURN;
            break;

        case USER_TURN:
            USER_STATE = PAUSED;
            tone_index = 0;
            sequence_matched = 1;
            state_lfsr = start_state_lfsr;

            while (1)
            {
                if (reset)
                {
                    break;
                }
                // Stop turn if user entered a wrong tone
                if (tone_index >= sequence_length || sequence_matched == 0)
                {
                    break;
                }

                // Previous debounced state
                pb_previous_state = pb_new_state;
                pb_new_state = pb_state;

                // Compare edge & prev/new state
                pb_falling_edge = (pb_previous_state ^ pb_new_state) & pb_previous_state; // pressed
                pb_rising_edge = (pb_previous_state ^ pb_new_state) & pb_new_state;       // released

                // This state machine encapsulates the different states
                // that happen during human player's turn
                switch (USER_STATE)
                {
                case PAUSED:
                    allow_updating_playback_delay = 1;

                    if (pb_falling_edge & PB1)
                    {
                        allow_updating_playback_delay = 0; // disable playback time update when pressed
                        play_tone(TONE_1);
                        display_tone(TONE_1);
                        pb_released = 0;
                        elapsed_time = 0;
                        user_input_index = 0;
                        USER_STATE = PLAYING;
                    }
                    else if (pb_falling_edge & PB2)
                    {
                        allow_updating_playback_delay = 0;
                        play_tone(TONE_2);
                        display_tone(TONE_2);
                        pb_released = 0;
                        elapsed_time = 0;
                        user_input_index = 1;
                        USER_STATE = PLAYING;
                    }
                    else if (pb_falling_edge & PB3)
                    {
                        allow_updating_playback_delay = 0;
                        play_tone(TONE_3);
                        display_tone(TONE_3);
                        pb_released = 0;
                        elapsed_time = 0;
                        user_input_index = 2;
                        USER_STATE = PLAYING;
                    }
                    else if (pb_falling_edge & PB4)
                    {
                        allow_updating_playback_delay = 0;
                        play_tone(TONE_4);
                        display_tone(TONE_4);
                        pb_released = 0;
                        elapsed_time = 0;
                        user_input_index = 3;
                        USER_STATE = PLAYING;
                    }
                    break;

                case PLAYING:
                    if (!pb_released) // if not released
                    {
                        if (pb_rising_edge & PB1 && user_input_index == 0) // now if button is released
                            pb_released = 1;
                        else if (pb_rising_edge & PB2 && user_input_index == 1)
                            pb_released = 1;
                        else if (pb_rising_edge & PB3 && user_input_index == 2)
                            pb_released = 1;
                        else if (pb_rising_edge & PB4 && user_input_index == 3)
                            pb_released = 1;
                    }
                    else // if released
                    {
                        if (elapsed_time >= playback_time && pb_released)
                        {
                            stop_tone();
                            clear_display();
                            allow_updating_playback_delay = 1;
                            next();
                            is_sequence_matched(user_input_index, next_lfsr_digit);
                            tone_index++;
                            USER_STATE = PAUSED; // register another user's input
                        }
                    }
                    break;
                default:
                    break;
                }
            }

            if (reset)
            {
                STATE = INIT;
                reset = 0;
                break;
            }

            STATE = RESULT;
            break;

        case RESULT:
            RESULT_STATE = STATUS;

            // Encapsulate the logic in displaying result state
            while (1)
            {
                if (reset)
                {
                    break;
                }

                // Stop the loop when user lost
                if (STATE == SIMON_TURN || STATE == HIGHSCORE_ENTRY)
                    break;

                switch (RESULT_STATE)
                {
                case STATUS:
                    // Check whether user win or lose
                    if (sequence_matched)
                    {
                        elapsed_time = 0;
                        display_success();
                        printf("SUCCESS\n");
                        user_score = sequence_length;
                        sequence_length++;
                    }
                    else
                    {
                        elapsed_time = 0;
                        display_fail();
                        printf("GAME OVER\n");
                        user_score = sequence_length - 1;
                        sequence_length = 1;
                        start_state_lfsr = state_lfsr;
                    }

                    RESULT_STATE = DISPLAY_STATUS;
                    break;

                case DISPLAY_STATUS:
                    if (elapsed_time >= playback_time)
                    {
                        elapsed_time = 0;
                        display_score(user_score);
                        printf("%d\n", user_score);

                        RESULT_STATE = DISPLAY_SCORE;
                    }
                    break;

                case DISPLAY_SCORE:
                    if (elapsed_time >= playback_time)
                    {
                        clear_display();
                    }
                    else
                    {
                        break;
                    }

                    // transition to checking for high score when user loses
                    if (sequence_length == 1)
                    {
                        STATE = HIGHSCORE_ENTRY;
                        HIGHSCORE_STATE = CHECK_SCORE;
                        break;
                    }

                    STATE = SIMON_TURN; // Restart the game
                    break;

                default:
                    break;
                }
            }

            if (reset)
            {
                STATE = INIT;
                allow_updating_playback_delay = 1;
                playback_time = 250 + ((1757UL * ADC0.RESULT) >> 8);
                _delay_ms(1);
                reset = 0;
                break;
            }
            break;

        case HIGHSCORE_ENTRY:
        {
            // Different states when user need to enter their scores
            switch (HIGHSCORE_STATE)
            {
            case CHECK_SCORE:
                if (!in_top_five(user_score)) // Not in top-five
                {
                    STATE = SIMON_TURN; // Restart the game
                    SERIAL_STATE = AWAITING_COMMAND;
                    break;
                }

                // Otherwise, start recording user's name
                SERIAL_STATE = AWAITING_NAME;
                HIGHSCORE_STATE = RECORD_ENTRY;
                current_index = 0; // keep track of current input index
                printf("Enter name:");
                break;

            case RECORD_ENTRY:
                elapsed_time = 0;
                HIGHSCORE_STATE = ENTERING;
                break;

            case ENTERING:
                // Check whether username is blank after 5 seconds
                if (current_index == 0)
                {
                    if (elapsed_time > 5000)
                    {
                        add_high_score("", user_score);
                        HIGHSCORE_STATE = DISPLAY_ENTRY;
                    }
                }
                else
                {
                    if (elapsed_time > 5000 || save_entry) // No input after 5 seconds
                    {
                        HIGHSCORE_STATE = DISPLAY_ENTRY;
                        add_high_score(name_entry, user_score);
                        save_entry = 0;
                        break;
                    }
                }

                break;
            case DISPLAY_ENTRY:
                display_high_scores();
                STATE = SIMON_TURN;
                SERIAL_STATE = AWAITING_COMMAND;
                break;
            };
        }
        break;

        default:
            break;
        }
    }
}
