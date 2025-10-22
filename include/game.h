#include <stdint.h>
//#include "sequence.h"
void update_buttons();
void play_sequence();
void set_outputs(uint8_t index);
void outputs_off();
void init_sys();

typedef enum {
    PROGRESS,
    DISPLAY_ON,
    DISPLAY_OFF,
    WAIT_INPUT,
    WAIT_RELEASE,
    HANDLE_INPUT,
    CORRECT,
    SUCCESS,
    FAIL,
    RESET,
    SEED 
} game_state;

extern volatile game_state state;
//extern volatile LFSR lfsr;
extern volatile char hex_seed[9];  
extern volatile uint8_t seed_ready;   // Flag when seed is complete
