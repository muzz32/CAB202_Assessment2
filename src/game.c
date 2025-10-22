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

uint8_t curr_button_state, prev_button_state = 0xFF;
uint8_t button_change, button_input, button_release = 0;
uint8_t input, curr_seq;
uint32_t new_seed;
LFSR lfsr;
volatile char hex_seed[9];  

volatile game_state state = PROGRESS;

int main(void){

    init_sys();

    //game_state state = PROGRESS;

    while (1)
    {
        update_buttons();
        switch (state)
        {
        case PROGRESS:
            if(lfsr.seed){
                lfsr.state = lfsr.seed;
            }
            lfsr.sequence_length++;
            lfsr.sequence_index = 0;
            state = DISPLAY;
            break;
        case DISPLAY:
            play_sequence();
            reset_lfsr(&lfsr);
            state = WAIT_INPUT;
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
                update_delay();
                set_outputs(input);
                elapsed_time = 0;  
                state = WAIT_RELEASE;
            }
            break;
        case WAIT_RELEASE:
            if(elapsed_time >= (playback_delay>>1) || button_release){
                outputs_off();  
                state=HANDLE_INPUT; 
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
                elapsed_time = 0;
                state = FAIL;
            }                        
            break;
        case SUCCESS:
            if (elapsed_time >= playback_delay)
            {
                outputs_off();
                reset_lfsr(&lfsr);
                state = PROGRESS;
            }
            
            break;
        case FAIL:
            if(elapsed_time >= playback_delay){
                outputs_off();
                update_delay();
                display_score(lfsr.sequence_length);
                elapsed_time = 0;
                while (elapsed_time<playback_delay);
                outputs_off(); 
                elapsed_time = 0;
                while (elapsed_time<playback_delay);
                lfsr.sequence_length = 0;
                lfsr.start_state = lfsr.state;
                state = PROGRESS;
            }
            break;
        case RESET:
            buzzer_init();
            lfsr_init(&lfsr);
            outputs_off();
            state = PROGRESS;
            break;
        case SEED:
            new_seed = strtoul((const char*)hex_seed, NULL, 16);
            lfsr.seed = new_seed;
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
    button_input =  button_change & curr_button_state;
    button_release = button_change & prev_button_state; 
}

void play_sequence(){
    uint8_t rand;
    update_delay();
    for(uint8_t i = 0; i < lfsr.sequence_length; i++){       
        rand = step(&lfsr);
        
        set_outputs(rand);
        elapsed_time = 0;  
        while (elapsed_time<(playback_delay>>1));
        outputs_off();
        elapsed_time = 0;  
        while (elapsed_time < (playback_delay>>1));
    }
}

void set_outputs(uint8_t index){
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
}

void outputs_off(){
    buzzer_off();
    set_display(DISP_OFF, DISP_OFF);
}

void init_sys(){
    cli();
    lfsr_init(&lfsr);
    disp_init();
    timer_init();
    buzzer_init();
    uart_init();
    button_init();
    adc_init();
    sei();
}