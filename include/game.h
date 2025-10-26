#ifndef GAME_H
#define GAME_H

#include <stdint.h>

#define TABLE_LENGTH 5
// Defines an enum to represent all relevant states of the game for a state machine
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
    SHOW_SCORE,
    HIDE_SCORE,
    GET_HIGHSCORE
} game_state;

/*
A simple struct to store a users name (20 digits plus null terminator) and 
their score.
*/ 
typedef struct{
    char name[21];
    uint16_t score;
}USER;

void game_init(USER *highscore_table);
void update_buttons();
void set_outputs(uint8_t index);
void outputs_off();
void init_sys();
void handle_new_user(USER new_user);
uint8_t check_scores(USER *highscore_table, uint16_t score);
void resort_list(USER new_user, uint8_t new_user_place, USER *highscore_table);
extern volatile game_state state; // External so uart can see when state == GET_HIGHSCHOOL

// All uint8 variables needed for the game
uint8_t input, curr_seq, button_is_released, curr_button_state, prev_button_state, button_change, button_release, button_input, randnum, display_index, score_check_res;




#endif