#ifndef GAME_H
#define GAME_H

#include <stdint.h>
//#include "sequence.h"

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
    SEED,
    GET_HIGHSCORE
} game_state;

typedef struct{
    char name[20];
    uint16_t score;
}USER;

void game_init(USER *highscore_table);
void update_buttons();
void set_outputs(uint8_t index);
void outputs_off();
void init_sys();
void check_scores(USER *highscore_table, uint16_t score);

extern volatile game_state state;
extern volatile game_state pre_seed_state;
//extern volatile LFSR lfsr;
 
extern volatile uint8_t seed_ready;   // Flag when seed is complete

#endif