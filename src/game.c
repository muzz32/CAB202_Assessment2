#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

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
//volatile char hex_seed[9];  

volatile game_state state = PROGRESS;
volatile game_state pre_seed_state;

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
            if(lfsr.seed){
                //printf("\nnew state");
                lfsr.state = lfsr.seed;
                //printf("%08lx", lfsr.state);
                lfsr.seed = 0;
            }
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
                while (elapsed_time<playback_delay);
                outputs_off(); 
                elapsed_time = 0;
                while (elapsed_time<playback_delay);

                uint8_t score_check_res = check_scores(&highscore_table, lfsr.sequence_length);
                if (score_check_res)
                {
                    state = GET_HIGHSCORE;
                }
                else{
                    lfsr.sequence_length = 0;
                    lfsr.start_state = lfsr.state;
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
        case SEED:
            //printf("seed state");
            if(seed_ready){
                new_seed = strtoul((const char*)hex_seed, NULL, 16);
                lfsr.seed = new_seed;
                //printf("%08lx", lfsr.seed);
            }
            state = pre_seed_state;
            break;
        case GET_HIGHSCORE:
            printf("Enter name: ");
            break;
        default:
            state = RESET;
            break;
        }
    }
}

void update_buttons(){
    prev_button_state = curr_button_state;
    curr_button_state = button_debounced;
    button_change = curr_button_state ^ prev_button_state;
    button_input = button_change & prev_button_state; 
    button_release =  button_change & curr_button_state;
}

void game_init(USER *highscore_table) {
    for(uint8_t i =0; i < (TABLE_LENGTH-1); i++){
        highscore_table[i] = empty_user;
    }
    button_is_released = 0;
    curr_button_state, prev_button_state = 0xFF;
}

uint8_t check_scores(USER *highscore_table, uint16_t score){
    uint8_t highscore_pos=0;
    for(uint8_t i = 1; i < TABLE_LENGTH; i++){
        if(highscore_table[i-1].score < score){
            highscore_pos = i;
            continue;
        }
        else if (highscore_pos){
            break;
        }
    }
    return highscore_pos;
}

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

void outputs_off(){
    buzzer_off();
    set_display(DISP_OFF, DISP_OFF);
}


void init_sys(){
    cli();
    game_init(&highscore_table);
    lfsr_init(&lfsr);
    disp_init();
    timer_init();
    buzzer_init();
    uart_init();
    button_init();
    adc_init();
    sei();
}