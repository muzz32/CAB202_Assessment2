#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint32_t mask = 0xE2025CAB;
uint32_t sid = 0x11958774;

typedef struct{
    uint32_t state;
    uint32_t start_state;
    uint32_t sequence_length;
    uint32_t sequence_index;
    uint32_t seed;
}LFSR;

uint8_t step(LFSR *lfsr){
    uint8_t lsb = lfsr->state & 1;
    lfsr->state >>= 1;
    if(lsb){
        lfsr->state ^= mask;
    }
    return lfsr->state & 0b11;
}

void reset_lfsr(LFSR *lfsr){
    lfsr->state = lfsr->start_state;
}

void lfsr_init(LFSR *lfsr){
    lfsr->start_state = sid;
    lfsr->state = lfsr->start_state;
    lfsr->sequence_length = 0;
    lfsr->sequence_index = 0;
    lfsr->seed = 0;
}

// void state_lfsr(){
//     uint8_t lsb = 
// }