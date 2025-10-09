#include <avr/io.h>
#include <avr/interrupt.h>
#include "button.h"
#include "display.h"

#define MAX_DELAY 2000
#define MIN_DELAY 250

uint16_t playback_delay = MIN_DELAY;
volatile uint16_t elapsed_time = 0;



void tca_init(){
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc; // Configure prescalar
    TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP1_bm |TCA_SINGLE_CMP0_bm| TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_WGMODE_SINGLESLOPE_gc; //Enable WO1 on PB1 and WO0 on PB0
    PORTB.OUTSET = PIN1_bm;
    PORTB.DIRSET = PIN1_bm | PIN0_bm;
    PORTB.OUTCLR = PIN0_bm;//Enable pins for disp_en
    TCA0.SINGLE.PER = 10000; //Set top
    TCA0.SINGLE.CMP0 = 0;
    TCA0.SINGLE.CMP1 = 0; //Set Compare
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; //Enable
}

void tcb_init(){
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Configure TCB1 in periodic interrupt mode
    TCB0.CCMP = 3333;                // Set interval for 1 ms (3333 clocks @ 3.333 MHz)
    TCB0.INTCTRL = TCB_CAPT_bm;      // CAPT interrupt enable
    TCB0.CTRLA = TCB_ENABLE_bm;      // Enable

    TCB1.CTRLB = TCB_CNTMODE_INT_gc; // Configure TCB1 in periodic interrupt mode
    TCB1.CCMP = 16667;               // Set interval for 5 ms (16667 clocks @ 3.333 MHz)
    TCB1.INTCTRL = TCB_CAPT_bm;      // CAPT interrupt enable
    TCB1.CTRLA = TCB_ENABLE_bm;      // Enable
}

void timer_init(){
    tca_init();
    tcb_init();
}

ISR(TCB0_INT_vect){
    elapsed_time ++;
    TCB0.INTFLAGS = TCB_CAPT_bm;
}

ISR(TCB1_INT_vect){
    get_debounce();
    swap_digit();
    TCB1.INTFLAGS = TCB_CAPT_bm;
}