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



void game_init(){
    sequence_length = 1;

}





