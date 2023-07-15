// Ensure that this file is only included once
#ifndef INIT_H
#define INIT_H 1

#include <avr/io.h>

#include <stdio.h>

void buttons_init(void)
{
    // Enable pull-up resistors for PBs
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

void port_init(void)
{

    // BUZZER (PIN0), USART0 TXD (PIN2)
    PORTB.DIRSET = PIN0_bm | PIN2_bm;
}

void pwm_init(void)
{
    PORTB.DIRSET = PIN0_bm;
    PORTB.OUTSET = PIN0_bm;

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;
}

void timers_init(void)
{
    // 1ms interrupt for elapsed time
    TCB0.CCMP = 3333;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_ENABLE_bm;

    // 10ms interrupt for PB sampling and display to 7-segments display
    TCB1.CCMP = 33333;
    TCB1.INTCTRL = TCB_CAPT_bm;
    TCB1.CTRLA = TCB_ENABLE_bm;
}

void uart_init(void)
{
    // 9600 baud
    USART0.BAUD = 1389;

    // Enable receive complete interrupt
    USART0.CTRLA = USART_RXCIE_bm;
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
}

void adc_init(void)
{
    ADC0.CTRLA = ADC_ENABLE_bm;
    ADC0.CTRLB = ADC_PRESC_DIV2_gc;
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp | ADC_REFSEL_VDD_gc);
    ADC0.CTRLE = 64;
    ADC0.CTRLF = ADC_FREERUN_bm | ADC_LEFTADJ_bm;

    // use AIN2 (POT)
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;

    // single ended 8-bit conversion
    ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc;
}

#endif
