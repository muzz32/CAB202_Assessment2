#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>

#define MIN_FREQ 167     // 20 kHz
#define MAX_FREQ 166650  // 20 Hz

// E_HIGH 9387
// C_SHARP 11163
// A 7032
// E_LOW 18774

/*
Array of the tones used in the game (Notes above). Already converted to timer periods to save computation.
*/
static uint32_t tops[4] = {9387, 11163, 7032, 18774};

/*
Initialises the tones of the buzzer to their defaults
*/
void buzzer_init(){
    tops[0] = 9387;
    tops[1] = 11163;
    tops[2] = 7032;
    tops[3] = 18774;
}


/*
Sets the compare value to 0 to turn off the buzzer
*/
void buzzer_off(){
    TCA0.SINGLE.CMP0BUF = 0;
}

/*
Plays one of 4 tones stored in the tops array on the buzzer
at a 50% duty cycle. If index is out of bounds, the buzzer is
turned off
*/
void set_buzzer(uint8_t index){
    if(index < 0 || index > 3){
        buzzer_off();
    }
    else{
        TCA0.SINGLE.PERBUF = tops[index]; // Period set to the tone in the array
        TCA0.SINGLE.CMP0BUF = tops[index] >> 1; // Compare set to half period for 50% duty cycle
        printf("%ld", tops[index]);
    }
}

/*
If the increase won't put the tone out of the frequency bounds,
the octave of every tone in the tops array is increased once.
*/
void increase_octave(){
    uint8_t safe_to_change = 1;
    for (uint8_t i = 0; i < 4; i++)
    {
        // If the value divided by two is less than the min freq, dont change
        if((tops[i] >> 1) > MAX_FREQ){
            safe_to_change = 0;
        } 
    }
    // If safe to change, increase the octave by halving each top value
    if(safe_to_change){
        for (uint8_t i = 0; i < 4; i++)
        {
            tops[i] >>= 1;
        }
    } 
}

/*
If the decrease won't put the tone out of the frequency bounds,
the octave of every tone in the tops array is decreased once.
*/
void decrease_octave(){
    uint8_t safe_to_change = 1;
    for (uint8_t i = 0; i < 4; i++)
    {
        // If the value multiplied by two is more than the max freq, dont change
        if((tops[i] << 1) < MIN_FREQ){
            safe_to_change = 0;
        } 
    }
    //if its safe to change, decrease the octave by doubling each top value
    if(safe_to_change){
        for (uint8_t i = 0; i < 4; i++)
        {
            tops[i] <<= 1;
        }
    } 
}