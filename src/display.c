#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "display.h"

//The spi data that gets transmitted for the left and right dig
uint8_t left_dig;
uint8_t right_dig;

// An array of digit bitmasks from 0 to 9. Used in display score.
static const uint8_t nums[10] = {DISP_ZERO, DISP_ONE, DISP_TWO, DISP_THREE, DISP_FOUR, DISP_FIVE, DISP_SIX, DISP_SEVEN, DISP_EIGHT, DISP_NINE};

void disp_init(){
    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc; //Alternate pin configuration that the QUTY uses on page 144
    PORTC.DIRSET = (PIN0_bm | PIN2_bm); //Enable SPI CLK and SPI MOSI (Master out Slave in) as outputs
    SPI0.CTRLA = SPI_MASTER_bm; // Enable this as the master 
    PORTA.OUTSET = PIN1_bm; // DISP_LATCH initial high
    PORTA.DIRSET = PIN1_bm; // set DISP_LATCH pin as output
    PORTB.OUTSET = PIN1_bm;
    PORTB.DIRSET = PIN1_bm; //Set display as output
    SPI0.CTRLB = SPI_SSD_bm; // Mode 0, client select disable, unbuffered
    SPI0.INTCTRL = SPI_IE_bm;  //Enable interrupts

    left_dig = DISP_OFF | DISP_LHS;
    right_dig = DISP_OFF; //Digits initially set to off bitmasks

    SPI0.CTRLA |= SPI_ENABLE_bm; // Enable SPI
    SPI0.DATA = right_dig; //Load the right_dig data into SPI0 (Currently off)
}

/*
Updates the left and right digits with two bitmasks. Doesn't immediately
load that data, rather it updates the date to be loaded by swap_digit
*/
void set_display(uint8_t left, uint8_t right){
    left_dig = left | DISP_LHS;
    right_dig = right;
}

/*
Takes in an unsigned 16bit integer, usually the users score and updates the left and right dig to
displays it. If the number is less that 10, the digit will be displayed on the right hand side with the
left side off. If its over 99, the last two numbers in the score will be displayed, the score being 
outlined as over 99 in the case of single digits left over by displaying a 0 on the right hand side. 
All other numbers will be displayed as expected.
*/
void display_score(uint16_t score){
    // If the score is over 100 (ie 3 digits), subtract 100 till its only 2 digits, and register that its over
    uint8_t over_hundred = 0;
    if(score>99){
        while (score >= 100) {
            score -= 100;
        }
        over_hundred = 1;
    }


    uint16_t left_num = 0; 
    uint16_t right_num = score; //temp storing
    while(left_num>=10){
        right_num = right_num - 10; //the remainder of the division by 10 (gives whats left over by dividing by 10, giving the right dig)
        left_num++; //Amount of times dig can be divided by 10 (gives first dig)
    }

    if(left_num == 0){
        if (over_hundred){
            // If its over 100 and 1 digit, set the left dig to 0 and the display the right number
            set_display(DISP_ZERO, nums[right_num]);
        }
        else{
            // If its not over 100 and its 1 digit, turn off the left dig and display the right one
            set_display(DISP_OFF, nums[right_num]);
        }
    }
    else{
        // Otherwise, display the left and right digits of their score
        set_display(nums[left_num], nums[right_num]);
    }
}

/*
Swaps what digit is being transfered to the display each time its ran. This
runs in an interrupt every 5ms, giving the illusion that both digits are on
*/
void swap_digit(){
    static int digit = 0;
    if (digit) {
        SPI0.DATA = left_dig; 
    } else {
        SPI0.DATA = right_dig; 
    }
    digit = !digit;
}

/*
This interrupt executes everytime data finishes being
transfered to the display. The latch is triggered when this
happens, which turns on the display.
*/
ISR(SPI0_INT_vect){
    //rising edge on DISP_LATCH
    PORTA.OUTCLR = PIN1_bm;
    PORTA.OUTSET = PIN1_bm;  
    SPI0.INTFLAGS = SPI_IF_bm;
}
