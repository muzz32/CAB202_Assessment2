#include <avr/io.h>
#include <avr/interrupt.h>
#include "button.h"
#include "display.h"
#include "uart.h"
#include <stdint.h>


volatile uint16_t elapsed_time; // Counter variable volatile due to it being increased by TCB0 ISR

/* 
Enables the TCA0 peripheral for use with the buzzer
*/
void tca_init(){
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc; // Configure prescalar of 1
    TCA0.SINGLE.CTRLB =TCA_SINGLE_CMP0_bm| TCA_SINGLE_WGMODE_SINGLESLOPE_gc; //Enable WO0 on PB0 (Buzzer pin) and use single slope mode
    PORTB.DIRSET = PIN0_bm;
    PORTB.OUTCLR = PIN0_bm; //Enable buzzer as output
    TCA0.SINGLE.PER = 0; 
    TCA0.SINGLE.CMP0 = 0; //Set initial period and compare to 0 (off)
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; //Enable timer A
}

/* 
Enables the TCB0 peripheral to trigger interrupts every 1ms
and TCB1 peripheral to trigger interrupts every 5ms
*/
void tcb_init(){
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Configure TCB0 in periodic interrupt mode
    TCB0.CCMP = 3333;                // Set interval for 1 ms (3333 clocks @ 3.333 MHz)
    TCB0.INTCTRL = TCB_CAPT_bm;      // CAPT interrupt enable
    TCB0.CTRLA = TCB_ENABLE_bm;      // Enable

    TCB1.CTRLB = TCB_CNTMODE_INT_gc; // Configure TCB1 in periodic interrupt mode
    TCB1.CCMP = 16667;               // Set interval for 5 ms (16667 clocks @ 3.333 MHz)
    TCB1.INTCTRL = TCB_CAPT_bm;      // CAPT interrupt enable
    TCB1.CTRLA = TCB_ENABLE_bm;      // Enable
}

/*
Initialises both timers and sets elapsed_time to 0
*/
void timer_init(){
    tca_init();
    tcb_init();
    elapsed_time = 0;
}

/*
Interrupt will trigger every 1ms and will increment the
elapsed_time variable. Used for delaying outputs.
*/
ISR(TCB0_INT_vect){
    elapsed_time ++;
    TCB0.INTFLAGS = TCB_CAPT_bm;
}

/*
Will trigger every 5ms and with swap the current digit of the display, 
and get the current button state.
*/
ISR(TCB1_INT_vect){
    swap_digit();
    get_debounce();
    TCB1.INTFLAGS = TCB_CAPT_bm;
}