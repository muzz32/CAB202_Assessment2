#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>

uint16_t freq;

  //370hz
    //14%
    //
    //F_pwm = 370
    //T_pwm = 1/370 = 2.7027ms
    //T_clk,timer = 300ns
    //TOP = T_pwm/T_clk,timer = 2.7027ms/300ns = 9009
    //Duty cycle = cmp/TOP
    //cmp = 0.14 * 9009 =   1622 

void e_high(){
    freq = 474 * pow(2, -5/12); //355.0987772 ... = 9387
    TCA0.SINGLE.PERBUF = 9387;
    TCA0.SINGLE.CMP0BUF = 9387/2;
}

void c_sharp(){
    freq = 474*pow(2, -8/12);
}

void a(){
    freq = 474;
}

void e_low(){
    freq = 474 * pow(1, -17/12);
}
