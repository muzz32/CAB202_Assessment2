#include <stdint.h>
#include <avr/io.h>
#include <stdio.h>

// Max and Min delay macros in case delay specifications change in the future
#define MAX_DELAY 2000
#define MIN_DELAY 250

volatile uint16_t playback_delay; // The global playback delay which is set between the MAX and MIN delay

void adc_init(){
    //Potentiometer on PA2
    ADC0.CTRLA = ADC_ENABLE_bm; // Enable the adc
    ADC0.CTRLB = ADC_PRESC_DIV2_gc; // Prescalar of 2 to maximise speed of conversion
    //Sets the number of clock cycles to get 1us and sets reference to the Vdd (0-3.3V)
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc;
    ADC0.CTRLF = ADC_FREERUN_bm | ADC_LEFTADJ_bm; //Freerun mode and left adjusted result
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc; // Read on the ain2 (pos) spot
    ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc; //Starts the conversions in single 8bit mode immediately
}

/*
Returns the current playback delay by reading the current ADC0 result, multiplying with the delay
range, dividing the result by 256 (8 right shifts) and then adding the MIN delay. This gives a result
between 250ms and 2s
*/
void update_delay(){
    //Cast to uint32_t incase of overflow
    playback_delay = MIN_DELAY + (((uint32_t)(MAX_DELAY - MIN_DELAY) * ADC0.RESULT) >> 8);
}