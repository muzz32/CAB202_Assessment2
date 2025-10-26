#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

#include "sequence.h"
#include "button.h"
#include "display.h"
#include "buzzer.h"
#include "timer.h"
#include "game.h"
#include "uart.h"
#include "delay.h"

#define TABLE_LENGTH 5

// typedef enum{
//     PROGRESS,
//     DISPLAY,
//     WAIT_INPUT,
//     WAIT_RELEASE,
//     HANDLE_INPUT,
//     CORRECT,
//     SUCCESS,
//     FAIL,
//     RESET    
// }game_state;
uint8_t button_is_released = 0;
uint8_t curr_button_state, prev_button_state = 0xFF;
uint8_t button_change, button_release, button_input = 0;
uint8_t input, curr_seq;
uint32_t new_seed;
LFSR lfsr;
uint8_t randnum, display_index = 0;
uint8_t score_check_res;
//volatile char hex_seed[9];  

volatile game_state state = PROGRESS;


USER highscore_table[TABLE_LENGTH];
USER empty_user = {
    .name = "\0",
    .score = 0
};


int main(void){

    init_sys();

    //game_state state = PROGRESS;

    while (1)
    {
        //update_delay();
        update_buttons();
        switch (state)
        {
        case PROGRESS:
            lfsr.sequence_length++;
            lfsr.sequence_index = 0;
            //update_delay();
            randnum = step(&lfsr);  
            set_outputs(randnum);
            state = DISPLAY_OFF;
            break;
        case DISPLAY_ON:
            if(display_index == lfsr.sequence_length){
                reset_lfsr(&lfsr);
                display_index = 0;
                state = WAIT_INPUT;
            }
            else if (elapsed_time >= (playback_delay>>1)){
                randnum = step(&lfsr);  
                set_outputs(randnum);
                state = DISPLAY_OFF;
            }
            break;
        case DISPLAY_OFF:
            if (elapsed_time >= (playback_delay>>1))
            {
                outputs_off();
                elapsed_time = 0;
                display_index++;
                state = DISPLAY_ON;
            }
            break;
        case WAIT_INPUT:
            if(button_input){
                if(button_input & PIN4_bm){
                    input = 0;
                }
                else if(button_input & PIN5_bm){
                    input = 1;
                }
                else if(button_input & PIN6_bm){
                    input = 2;
                }
                else if(button_input & PIN7_bm){
                    input = 3;
                }
                //update_delay();
                set_outputs(input);
                elapsed_time = 0;  
                state = WAIT_RELEASE;
                button_is_released = 0;
            }
            else if(uart_input_recieved && (uart_input >= 0 && uart_input <= 3)){
                input = uart_input;
                set_outputs(input);
                elapsed_time = 0;  
                state = WAIT_RELEASE;
                uart_input_recieved = 0;
                button_is_released = 1; //Skip button check and go straight to time check
            }
            break;
        case WAIT_RELEASE:
            if(!button_is_released){
                if(button_release) button_is_released = 1;
            }
            else{
                if (elapsed_time >= (playback_delay>>1))
                {
                    button_is_released = 0;
                    outputs_off();
                    state = HANDLE_INPUT;
                }
            }
            break;
        case HANDLE_INPUT:
            curr_seq = step(&lfsr);
            lfsr.sequence_index++;
            if(curr_seq == input)
            {
                if(lfsr.sequence_index == lfsr.sequence_length){
                    update_delay();
                    set_display(SUCCESS_PATTERN, SUCCESS_PATTERN);
                    printf("SUCCESS\n");
                    printf("%u\n", lfsr.sequence_length);
                    elapsed_time = 0;
                    state = SUCCESS;
                }
                else if(lfsr.sequence_index < lfsr.sequence_length){
                    state = WAIT_INPUT;
                }
            }
            else{
                update_delay();
                set_display(FAIL_PATTERN, FAIL_PATTERN);
                printf("GAME OVER\n");
                printf("%u\n", lfsr.sequence_length);
                elapsed_time = 0;
                state = FAIL;
            }                        
            break;
        case SUCCESS:
            if (elapsed_time >= playback_delay)
            {
                outputs_off();
                //elapsed_time = 0;
                reset_lfsr(&lfsr);
                state = PROGRESS;
            }
            
            break;
        case FAIL:
            if(elapsed_time >= playback_delay){
                outputs_off();
                //elapsed_time = 0;
                update_delay();
                display_score(lfsr.sequence_length);
                elapsed_time = 0;
                state = SHOW_SCORE;                
            }
            break;
        case SHOW_SCORE:
            if (elapsed_time >= playback_delay)
            {
                outputs_off(); 
                elapsed_time = 0;
                state = HIDE_SCORE;
            }
            break;
        case HIDE_SCORE:
            if (elapsed_time >= playback_delay)
            {
                score_check_res = check_scores(highscore_table, lfsr.sequence_length);
                if (score_check_res)
                {
                    printf("Enter name: ");
                    elapsed_time = 0;
                    state = GET_HIGHSCORE;
                }
                else{
                    lfsr.sequence_length = 0;
                    set_start_lfsr(&lfsr);
                    state = PROGRESS;
                }            
            }            
            break;
        case RESET:
            buzzer_init();
            lfsr_init(&lfsr);
            outputs_off();
            state = PROGRESS;
            break;
        case GET_HIGHSCORE:
            if(name_ready){
                USER new_user;
                if(name_index==20){
                    strncpy(new_user.name, (const char*)temp_name, 20);
                    new_user.name[21] = '\0';
                }
                else{
                    strncpy(new_user.name, (const char*)temp_name, name_index);
                    new_user.name[name_index] = '\0';
                }
                new_user.score = lfsr.sequence_length;
                handle_new_user(new_user);
                state = PROGRESS;
            }
            else{
                if (name_input_received)
                {
                    elapsed_time = 0;
                    name_input_received = 0;
                }
                else if(elapsed_time >= 5000){
                    USER new_user = empty_user;
                    if (name_index)
                    {
                        temp_name[name_index] = '\0';
                        strncpy(new_user.name, (const char*)temp_name, name_index);
                        new_user.score = lfsr.sequence_length;
                    }
                    else{
                        new_user.score = lfsr.sequence_length;
                    }
                    handle_new_user(new_user);
                    state = PROGRESS;
                }
            }
            break;
        default:
            state = RESET;
            break;
        }
    }
}

void handle_new_user(USER new_user){
    resort_list(new_user, score_check_res, highscore_table);
    score_check_res = 0;
    name_ready = 0;
    print_user_table(highscore_table, TABLE_LENGTH);
    lfsr.sequence_length = 0;
    set_start_lfsr(&lfsr);
    name_index = 0;
    uart_input_recieved = 0;  // dont pickup old inputs
    name_input_received = 0;
}

/*
Updates the button_input(falling edge) and button_release(rising edge)
variables by reading from button debounced (Set in ISR in timer.c by using
update_buttons in button.c)
*/
void update_buttons(){
    //Store previous state before overwriting current state
    prev_button_state = curr_button_state; 
    // Get the current debounced button state
    curr_button_state = button_debounced;
    // Get what changed by xoring the current state with the previous state
    button_change = curr_button_state ^ prev_button_state;
    // Inputs are where falling edges are detected on the pins (so and with the change and the previous state)
    button_input = button_change & prev_button_state; 
    // Releasing the button can be detected where there is a rising edge on a pin (so and button changes with the current state)
    button_release =  button_change & curr_button_state;
}

/*
Ensures that all game related variables are set to a safe
initial state.
*/
void game_init(USER *highscore_table) {
    // Overwrite all entries in the highscore table with blank users
    for(uint8_t i =0; i < (TABLE_LENGTH-1); i++){
        highscore_table[i] = empty_user;
    }
    button_is_released = 0;
    curr_button_state = 0xFF;
    prev_button_state = 0xFF;
    score_check_res = 0;
}

/*
Used in the HIDE_SCORE state to check if a user should be added
to the highscore table.  Will return a 0 if their score isnt in the
top 5 spots. If they are in the top five, It'll return their highscore position
(1 for 1st, 2 for 2nd and so on)
*/
uint8_t check_scores(USER *highscore_table, uint16_t score){
    uint8_t highscore_pos=0;
    /* For each user (starting at 1st place) in the highscore_table,
       if the score given to the function is higher than that users score,
       return the current position in the table.
    */
    for(uint8_t i = 1; i <= TABLE_LENGTH; i++){
        if(highscore_table[i-1].score < score){
            highscore_pos = i;
            return highscore_pos;
        }
    }
    // If no score is less than the entered score, the function will return 0
    return highscore_pos;
}

/*
Sorts the highscore_table of Users to accomodate a new user and their 
position in the leaderboard. 
*/
void resort_list(USER new_user, uint8_t new_user_place, USER *highscore_table){
    if(new_user_place == 5){
        highscore_table[4] = new_user; //Store new user in 5th, no sort needed
    }
    else{
        // Null index the users place (1st becomes 0, 2nd becomes 1 and so on)
        uint8_t new_user_index = new_user_place - 1;

        /*
        Starting at the lowest position in the array, each
        user is shifted down one position until i equals the
        new_user_place
        */
        for(uint8_t i = 4; i>new_user_index; i--)
        {
            highscore_table[i] = highscore_table[i-1];
        }
        // Insert the new user into the correct position in the array
        highscore_table[new_user_index] = new_user;
    }    
}


/*
Given a number from 0-3. this function outputs the correct bar 
on the display and tone on the buzzer. It also updates the delay 
at the start and resets the elapsed_time variable at the end.
*/
void set_outputs(uint8_t index){
    update_delay();
    switch (index)
    {
    case 0:
        set_display(DISP_BAR_1, DISP_OFF);
        break;
    case 1:
        set_display(DISP_BAR_2, DISP_OFF);
        break;
    case 2:
        set_display(DISP_OFF, DISP_BAR_1);
        break;
    case 3:
        set_display(DISP_OFF, DISP_BAR_2);
        break;
    default:
        set_display(DISP_OFF, DISP_OFF);
        break;
    }
    set_buzzer(index);
    elapsed_time = 0;
}

/*
Turns off the display and buzzer
*/
void outputs_off(){
    buzzer_off();
    set_display(DISP_OFF, DISP_OFF);
}

/*
Uses all init functions for interfaces relevant to setting up
the system for gameplay
*/
void init_sys(){
    cli(); // Pauses global interrupts
    game_init(highscore_table);
    lfsr_init(&lfsr); // Passes the address of the lfsr struct assigned in this file
    disp_init();
    timer_init();
    buzzer_init();
    uart_init();
    button_init();
    adc_init();
    sei(); // Enables global interrupts
}