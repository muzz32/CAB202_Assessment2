#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "display.h"

uint32_t init_state = 0x11958774;
uint32_t sequence_length;

void lfsr_init(){

}

void pushbut_init(){
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

void game_init(){
    sequence_length = 1;

}

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



void pwm_init(){
   //cli(); // Disable global intterupts
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc; // Configure prescalar
    TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP1_bm |TCA_SINGLE_CMP0_bm| TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_WGMODE_SINGLESLOPE_gc; //Enable WO1 on PB1 and WO0 on PB0
    PORTB.OUTSET = PIN1_bm;
    PORTB.DIRSET = PIN1_bm | PIN0_bm;
    PORTB.OUTCLR = PIN0_bm;//Enable pins for disp_en
    TCA0.SINGLE.PER = 10000; //Set top
    TCA0.SINGLE.CMP0 = 0;
    TCA0.SINGLE.CMP1 = 0; //Set Compare
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; //Enable
    //sei();
}