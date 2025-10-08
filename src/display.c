#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "display.h"
volatile uint8_t left_dig;
volatile uint8_t right_dig;

//     _____
//    |  A  |
//   F|     |B
//    |_____|
//    |  G  |
//   E|     |C
//    |_____|   
//       D  
                        //xFABGCDE
void disp_init(){
    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc; //Configuration that the QUTY uses on page 144

    PORTC.DIRSET = (PIN0_bm | PIN2_bm); //Enable SPI CLK and SPI MOSI (Master out Slave in) as outputs
    SPI0.CTRLA = SPI_MASTER_bm; // Enable this as the master 
    PORTA.OUTSET = PIN1_bm; // DISP_LATCH initial high
    PORTA.DIRSET = PIN1_bm; // set DISP_LATCH pin as output
    //SPI0.CTRLB &= ~SPI_BUFEN_bm; //disable buffer mode
    SPI0.CTRLB = SPI_SSD_bm;       // Mode 0, client select disable, unbuffered
    SPI0.INTCTRL = SPI_IE_bm;  //Enable interrupts

    left_dig = disp_off | disp_lhs;
    right_dig = disp_off;

    SPI0.CTRLA |= SPI_ENABLE_bm;

}

void set_display(uint8_t left, uint8_t right){
    left_dig = left | disp_lhs;
    right_dig = right;
}



void swap_digit(){
    static int digit = 0;
    if (digit) {
        SPI0.DATA = left_dig; 
    } else {
        SPI0.DATA = right_dig; 
    }
    digit = !digit;
}

ISR(SPI0_INT_vect){
    //rising edge on DISP_LATCH
    PORTA.OUTCLR = PIN1_bm;
    PORTA.OUTSET = PIN1_bm;  

    SPI0.INTFLAGS = SPI_IF_bm;
}
