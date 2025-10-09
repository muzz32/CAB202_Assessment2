#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>

#define E_HIGH 9387
#define C_SHARP 11163
#define A 7032
#define E_LOW 2634

static const uint16_t tops[4] = {E_HIGH, C_SHARP, A, E_LOW};

void set_buzzer(uint8_t index){
    TCA0.SINGLE.PERBUF = tops[index];
    TCA0.SINGLE.CMP0BUF = tops[index] >> 1;
}

void buzzer_off(){
    TCA0.SINGLE.CMP0BUF = 0;
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
