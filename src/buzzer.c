#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>

uint16_t freq;

void e_high(){
    freq = 474 * pow(2, -5/12);
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
