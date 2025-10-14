#include <stdint.h>

typedef struct{
    uint32_t state;
    uint32_t start_state;
    uint32_t sequence_length;
    uint32_t sequence_curr;
}LFSR;

void lfsr_init(LFSR *lfsr);

uint8_t step(LFSR *lfsr);

void reset_lfsr(LFSR *lfsr);