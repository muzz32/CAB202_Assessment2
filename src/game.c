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


volatile game_state state;
LFSR lfsr;

// The highscore table, which is an array of USERS length 5
USER highscore_table[TABLE_LENGTH];
// A user with no score and no name, used for reseting the table and temporarily adding to it
USER empty_user = {
    .name = "\0",
    .score = 0
};

int main(void){

    init_sys();

    while (1)
    {
        update_buttons();
        switch (state)
        {
        case PROGRESS:
            /*
            In the progress state, the sequence length is increased (initialised at 0) and the
            sequence index is reset. The first step of the sequence is played, and the state
            gets changed to DISPLAY OFF
            */
            lfsr.sequence_length++;
            lfsr.sequence_index = 0;
            randnum = step(&lfsr);  
            set_outputs(randnum);
            state = DISPLAY_OFF;
            break;
        case DISPLAY_ON:
            /*
            If all steps in the sequence have been played (ie display_index == sequence length),
            the lfsr is reset back to its start state and the state is changed to WAIT_INPUT
            */
            if(display_index == lfsr.sequence_length){
                reset_lfsr(&lfsr);
                display_index = 0;
                state = WAIT_INPUT;
            }
            /*
            If the sequence isn't finished, and the display has been off for half the playback 
            delay, display the next step and go back to DISPLAY_OFF
            */
            else if (elapsed_time >= (playback_delay>>1)){
                randnum = step(&lfsr);  
                set_outputs(randnum);
                state = DISPLAY_OFF;
            }
            break;
        case DISPLAY_OFF:
            /*
            Once half the playback delay has elapsed, the display is turned off, 
            the display index is increased and the state changes to DISPLAY_ON
            */
            if (elapsed_time >= (playback_delay>>1))
            {
                outputs_off();
                elapsed_time = 0;
                display_index++;
                state = DISPLAY_ON;
            }
            break;
        case WAIT_INPUT:
            /*
            The WAIT_INPUT state handles both UART and physical inputs. If a button input is
            detected, the input is set to what ever button was pressed (Found by anding the 
            button_input with the bitmasks of each of the buttons), that input is displayed
            and the state gets changed to the WAIT_RELEASE state
            */
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
                else{state = RESET;}
                set_outputs(input);
                state = WAIT_RELEASE;
                button_is_released = 0;
            }
            /*
            If a uart input has been recieved and its a valid input, the input is set to that,
            its displayed and the state is changed to WAIT_RELEASE. The uart_input_recieved 
            variable is set to zero so the input doesnt get used again, and the button_is_released
            variable is set to ignore checks for button rising edges.
            */
            else if(uart_input_recieved && (uart_input >= 0 && uart_input <= 3)){
                input = uart_input;
                set_outputs(input);
                state = WAIT_RELEASE;
                uart_input_recieved = 0;
                button_is_released = 1; //Skip button check and go straight to time check
            }
            break;
        case WAIT_RELEASE:
            /*
            This state ensures that button presses, if physical, are on for either the entirety
            of the button press, or half the playback delay (depending on which is longer). If
            its a UART input, the input just has to be displayed for half the playback delay.

            If the button_is_release variable isnt set, wait for a 
            button rising edge (set by button_release) and then set it
            */
            if(!button_is_released){
                if(button_release) button_is_released = 1;
            }
            /*
            If the button is released, and half the playback delay has elapsed,
            clear the button_is_released variable, turn off the output and go to
            the HANDLE_INPUT state
            */
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
            /*
            Sets the curr_seq to the current step of the shift register.
            If the input is the same as the curr_seq, the user is correct,
            otherwise they're wrong
            */
            curr_seq = step(&lfsr);
            lfsr.sequence_index++;
            if(curr_seq == input)
            {
                /*
                If the sequence index is equal to the length, the sequence is complete
                and the user is successful. Display the success pattern, and print out
                a success message and their score. Then, go to the SUCCESS state
                */
                if(lfsr.sequence_index == lfsr.sequence_length){
                    update_delay();
                    set_display(SUCCESS_PATTERN, SUCCESS_PATTERN);
                    elapsed_time = 0;
                    printf("SUCCESS\n");
                    printf("%u\n", lfsr.sequence_length);
                    state = SUCCESS;
                }
                /*
                If the sequence index is less than the length, the sequence isn't complete,
                so go back to WAIT_INPUT and get the next one
                */
                else if(lfsr.sequence_index < lfsr.sequence_length){
                    state = WAIT_INPUT;
                }
            }
            /*
            If the user is incorrect, display the fail pattern, print out a game over message and
            their final score, and then go to the FAIL state
            */
            else{
                update_delay();
                set_display(FAIL_PATTERN, FAIL_PATTERN);
                elapsed_time = 0;
                printf("GAME OVER\n");
                printf("%u\n", lfsr.sequence_length);
                state = FAIL;
            }                        
            break;
        case SUCCESS:
            /*
            Turn the success patter off after the playback delay has elapsed,
            reset the lfsr to prepare for the sequence to be displayed, and go
            back to the PROGRESS state
            */
            if (elapsed_time >= playback_delay)
            {
                outputs_off();
                reset_lfsr(&lfsr);
                state = PROGRESS;
            }
            
            break;
        case FAIL:
            /*
            Once the fail pattern has been displayed for the playback delay, turn it off, display their
            score and go to the SHOW_SCORE state
            */
            if(elapsed_time >= playback_delay){
                outputs_off();
                update_delay();
                display_score(lfsr.sequence_length);
                elapsed_time = 0;
                state = SHOW_SCORE;                
            }
            break;
        case SHOW_SCORE:
            /*
            Once the playback delay is elapsed, turn it off and go to the HIDE_SCORE state
            */
            if (elapsed_time >= playback_delay)
            {
                outputs_off(); 
                elapsed_time = 0;
                state = HIDE_SCORE;
            }
            break;
        case HIDE_SCORE:
            /*
            Once the display has been off for the playback delay, check if the users score is 
            is the top 5 highest scores (the highscore_table). If it is, prompt the user to
            enter their name and go to the GET_HIGHSCORE state. Otherwise, start a new game by 
            resetting the start state of the lfsr and going back to PROGRESS
            */
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
                    elapsed_time = 0;
                    state = PROGRESS;
                }            
            }            
            break;
        case GET_HIGHSCORE:
            /*
            This state gets a new users name and adds it to the highscore table.

            If the users name is finished (Signaled when the enter key is pressed)
            and its 20 characters long, copy all the characters in temp_name to the
            new users name.

            If its not 20 characters long, copy the amount of characters entered (plus one) and
            null terminate it accordingly. 

            Then, assign the users score to the lfsr sequence length, handle the new user, and
            start a new game by going back to the PROGRESS state
            */
            if(name_ready){
                USER new_user;
                if(name_index==20){
                    strncpy(new_user.name, (const char*)temp_name, 21);
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
            /*
            If the name isn't complete, check if an input has been recieved. If an input for the name
            has been recieved, reset the timer and clear the name_input_recieved variable.

            If an input hasnt been received and its been 5 seconds, Assign the users name to what ever
            has been entered so far. If no name has been entered (ie name_index == 0), just leave the name empty
            and assign the current score to that empty user. Handle the user like above, and then start a new game
            by going back to PROGRESS
            */
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
        case RESET:
            /*
            This is the games default state incase anything goes wrong. A user can also
            go to this state by entering the appropriate UART input. It will reset all 
            tones of the buzzer, re-initialise the lfsr, turn everything off, and then
            go to PROGRESS
            */
            buzzer_init();
            lfsr_init(&lfsr);
            outputs_off();
            state = PROGRESS;
            break;
        default:
            state = RESET;
            break;
        }
    }
}


/*
Contains all code relevant to adding a new user to the highscore table.
It first resorts the new_user into the highscore table, and then prints
that table out. It then resets all the relavent variables that were used,
including the temp_name, and also sets the lfsr start point.
*/
void handle_new_user(USER new_user){
    resort_list(new_user, score_check_res, highscore_table);
    print_user_table(highscore_table, TABLE_LENGTH);
    score_check_res = 0;
    name_ready = 0;
    lfsr.sequence_length = 0;
    set_start_lfsr(&lfsr);
    name_index = 0;
    uart_input_recieved = 0;  // dont pickup old inputs
    name_input_received = 0;
    temp_name[0] = '\0';  
    elapsed_time = 0;
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
    button_change= 0;
    button_release = 0;
    button_input = 0;
    randnum = 0;
    display_index = 0;
    state = PROGRESS;
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