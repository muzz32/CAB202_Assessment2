#include <stdint.h>
//#include "sequence.h"
void update_buttons();
void play_sequence();
void set_outputs(uint8_t index);
void outputs_off();
void init_sys();

typedef enum {
    PROGRESS,
    DISPLAY,
    WAIT_INPUT,
    WAIT_RELEASE,
    HANDLE_INPUT,
    CORRECT,
    SUCCESS,
    FAIL,
    RESET    
} game_state;

extern volatile game_state state;
