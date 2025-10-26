#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"


uint32_t mask = 0xE2025CAB;      // The mask used in the step function
uint32_t seq_seed = 0x11958774;  // The seed of the lfsr. Gets changed by the uart SEED command

/*
Defines a new structure with all lfsr
relavent variable types
*/
typedef struct{
    uint32_t state;
    uint32_t start_state;
    uint16_t sequence_length;
    uint16_t sequence_index;
}LFSR;

/*
Uses the lfsr to generate a random number between 0 and 3.
takes a pointer to an LFSR struct
*/
uint8_t step(LFSR *lfsr){
    uint8_t lsb = lfsr->state & 1; // Gets the least significant bit of the state
    lfsr->state >>= 1; // Shift the state right once
    if(lsb){
        lfsr->state ^= mask; //xor the state with the mask if the lsb is 1
    }
    return lfsr->state & 0b11; // Return the least 2 significant bits of the state, giving a number between 0-3
}

/*
Sets the state and start state of a given lfsr to the current seq_seed
*/
void update_state(LFSR *lfsr){
    lfsr->state = seq_seed;
    lfsr->start_state = seq_seed;
}


/*
Resets the state of the lfsr back to the start state 
(usually after the sequence is played). If a new seed
has been entered through UART, applies it here as well.
*/
void reset_lfsr(LFSR *lfsr){
    if(seed_ready){
        // Update the lfsr state to match the new seed and ensure it doenst get set again
        update_state(lfsr);
        seed_ready = 0;
    }
    else{
        lfsr->state = lfsr->start_state;
    }
}


/*
Updates the start_state of a given lfsr to the current state. Used after the user
fails to enter the sequence correctly. Has the same seed updating logic as
reset_lfsr
*/
void set_start_lfsr(LFSR *lfsr){
    if(seed_ready){
        update_state(lfsr);
        seed_ready = 0;
    }
    else{
        lfsr->start_state = lfsr->state;
    }
}


/*
Initialises a given lfsr by updating its state, 
and sets the sequence_length and sequence_index to 0 
*/
void lfsr_init(LFSR *lfsr){
    update_state(lfsr);
    lfsr->sequence_length = 0;
    lfsr->sequence_index = 0;
}