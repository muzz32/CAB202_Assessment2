#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "display.h"
uint8_t left_dig;
uint8_t right_dig;
static const uint8_t nums[10] = {DISP_ZERO, DISP_ONE, DISP_TWO, DISP_THREE, DISP_FOUR, DISP_FIVE, DISP_SIX, DISP_SEVEN, DISP_EIGHT, DISP_NINE};

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
    PORTB.OUTSET = PIN1_bm;
    PORTB.DIRSET = PIN1_bm; //Set display as out put
    //SPI0.CTRLB &= ~SPI_BUFEN_bm; //disable buffer mode
    SPI0.CTRLB = SPI_SSD_bm;       // Mode 0, client select disable, unbuffered
    SPI0.INTCTRL = SPI_IE_bm;  //Enable interrupts

    left_dig = DISP_OFF | DISP_LHS;
    right_dig = DISP_OFF;

    SPI0.CTRLA |= SPI_ENABLE_bm;
    SPI0.DATA = right_dig;
}

void set_display(uint8_t left, uint8_t right){
    left_dig = left | DISP_LHS;
    right_dig = right;
}

//https://stackoverflow.com/questions/21074682/dividing-a-number-without-using-division-operator-in-c
void display_score(uint8_t score){
    
    uint8_t over_hundred = 0;
    if(score>99){
        score-=100;
        over_hundred = 1;
    }

    uint8_t left_num = 0; 
    uint8_t right_num = score; //temp storing
    while(left_num>=10){
        right_num = right_num - 10; //the remainder of the division by 10 (gives whats left over by dividing by 10, giving the right dig)
        left_num++; //Amount of times dig can be divided by 10 (gives first dig)
    }

    if(left_num == 0){
        if (over_hundred){
            set_display(DISP_ZERO, nums[right_num]);
        }
        else{
            set_display(DISP_OFF, nums[right_num]);
        }
    }
    else{
        set_display(nums[left_num], nums[right_num]);
    }

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
