#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint32_t mask = 0xE2025CAB;

uint8_t step(state){
    uint8_t lsb = state & 1;
    state >>= 1;
    if(lsb){
        state ^= mask;
    }
    return state & 0b11;
}

// void state_lfsr(){
//     uint8_t lsb = 
// }