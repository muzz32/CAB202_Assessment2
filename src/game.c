#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "sequence.h"
#include "button.h"
#include "display.h"
#include "buzzer.h"

typedef enum{
    GENERATE,
    DISPLAY,
    WAIT_INPUT,
    WAIT_RELEASE,
    HANDLE_INPUT,
    SUCCESS,
    FAIL    
}game_state;

uint8_t curr_button_state, prev_button_state = 0xFF;
uint8_t button_change, button_input = 0;

LFSR lfsr;


int main(void){

    init_sys();

    game_state state = GENERATE;

    while (1)
    {
        update_buttons();
        switch (state)
        {
        case GENERATE:
            reset_lfsr(&lfsr);
            if(lfsr.sequence_length > 1){
               lfsr.sequence_length++; 
            }
            break;
        case DISPLAY:
            break;
        case WAIT_INPUT:
            break;
        case WAIT_RELEASE:
            break;
        case HANDLE_INPUT:
            break;
        case SUCCESS:
            break;
        case FAIL:
            break;
        default:
            state = GENERATE;
            break;
        }
    }
}

void update_buttons(){
    prev_button_state = curr_button_state;
    curr_button_state = button_debounced;
    button_change = curr_button_state ^ prev_button_state;
    button_input =  button_change & curr_button_state;
}

void play_sequence(){
    uint8_t rand;
    for(uint8_t i = 0; i < lfsr.sequence_length; i++){
        step(&lfsr);
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
    sei();
}