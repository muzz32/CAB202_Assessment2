#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint32_t mask = 0xE2025CAB;
uint32_t sid = 0x11958774;
typedef struct{
    uint32_t state;
    int sequence_length;
}LFSR;


uint8_t step(LFSR shiftr){
    uint8_t lsb = shiftr.state & 1;
    shiftr.state >>= 1;
    if(lsb){
        shiftr.state ^= mask;
    }
    return shiftr.state & 0b11;
}

LFSR lfsr_init(LFSR lfsr){
    lfsr.state = sid;
    lfsr.sequence_length = 1;
    return lfsr;
}

// void state_lfsr(){
//     uint8_t lsb = 
// }