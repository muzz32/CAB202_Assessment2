#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <stdint.h>

typedef struct{
    uint32_t state;
    uint32_t start_state;
    uint16_t sequence_length;
    uint16_t sequence_index;
}LFSR;

void lfsr_init(LFSR *lfsr);

uint8_t step(LFSR *lfsr);

void reset_lfsr(LFSR *lfsr);
void set_start_lfsr(LFSR *lfsr);
extern uint32_t seq_seed;

#endif