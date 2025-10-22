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
// E_LOW 2634

static uint32_t tops[4] = {9387, 11163, 7032, 2634};

void buzzer_init(){
    tops[0] = 9387;
    tops[1] = 11163;
    tops[2] = 7032;
    tops[3] = 2634;
}

void set_buzzer(uint8_t index){
    TCA0.SINGLE.PERBUF = tops[index];
    TCA0.SINGLE.CMP0BUF = tops[index] >> 1;
}

void buzzer_off(){
    TCA0.SINGLE.CMP0BUF = 0;
}

void increase_octave(){
    uint8_t unsafe_to_change = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        // If the value divided by two is less than the min freq, dont change
        if((tops[i] << 1) < MIN_FREQ){
            unsafe_to_change = 1;
        } 
    }
    if(!unsafe_to_change){
        for (uint8_t i = 0; i < 4; i++)
        {
            tops[i] >>= 1;
        }
    } 
}

void decrease_octave(){
    uint8_t unsafe_to_change = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        // If the value multiplied by two is more than the max freq, dont change
        if((tops[i] >> 1) < MAX_FREQ){
            unsafe_to_change = 1;
        } 
    }
    if(!unsafe_to_change){
        for (uint8_t i = 0; i < 4; i++)
        {
            tops[i] <<= 1;
        }
    } 
}



// uint16_t freq;

//   //370hz
//     //14%
//     //
//     //F_pwm = 370
//     //T_pwm = 1/370 = 2.7027ms
//     //T_clk,timer = 300ns
//     //TOP = T_pwm/T_clk,timer = 2.7027ms/300ns = 9009
//     //Duty cycle = cmp/TOP
//     //cmp = 0.14 * 9009 =   1622 

// void e_high(){
//     freq = 474 * pow(2, -5/12); //355.0987772 ... = 9387
//     TCA0.SINGLE.PERBUF = 9387;
//     TCA0.SINGLE.CMP0BUF = 9387/2;
// }

// void c_sharp(){
//     freq = 474*pow(2, -8/12); //top = 11163.1578901
// }

// void a(){
//     freq = 474; //top = 7032.3488045
// }

// void e_low(){
//     freq = 474 * pow(1, -17/12); //top = 2634.15449521 
// }
