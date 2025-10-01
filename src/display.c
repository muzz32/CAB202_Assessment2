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


void set_display(uint8_t left, uint8_t right){
    left_dig = left | disp_lhs;
    right_dig = right;
}

ISR(SPI0_INT_vect){
    //rising edge on DISP_LATCH
    PORTA.OUTCLR = PIN1_bm;
    PORTA.OUTSET = PIN1_bm;  

    SPI0.INTFLAGS = SPI_IF_bm;
}
