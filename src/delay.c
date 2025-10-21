#include <stdint.h>
#include <avr/io.h>
#include <stdio.h>

#define MAX_DELAY 2000
#define MIN_DELAY 250
volatile uint16_t playback_delay;

void adc_init(){

    //POT on PA2
    ADC0.CTRLA = ADC_ENABLE_bm; // Enable the adc
    ADC0.CTRLB = ADC_PRESC_DIV2_gc; //give it 16 coz why not
    //Sets the number of clock cycles to get 1us and sets reference to the Vdd (0-3.3V)
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc; 
    ADC0.CTRLF = ADC_FREERUN_bm; //Freerun mode
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc; // Read on the ain2 (pos) spot
    ADC0.COMMAND = ADC_MODE_SINGLE_12BIT_gc | ADC_START_IMMEDIATE_gc;
}

void update_delay(){
    playback_delay = MIN_DELAY + (((uint16_t)(MAX_DELAY - MIN_DELAY) * ADC0.RESULT) >> 8);
    //printf("%d\n", playback_delay);
}