#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#define DISP_LHS        0b10000000
#define DISP_OFF        0b01111111
#define DISP_BAR_1      0b00111110
#define DISP_BAR_2      0b01101011 
#define SUCCESS_PATTERN 0b00000000
#define FAIL_PATTERN    0b01110111

#define DISP_ZERO       0b00001000    
#define DISP_ONE        0b01101011   
#define DISP_TWO        0b01000100    
#define DISP_THREE      0b01000001  
#define DISP_FOUR       0b00100011    
#define DISP_FIVE       0b00010001    
#define DISP_SIX        0b00010000    
#define DISP_SEVEN      0b01001011   
#define DISP_EIGHT      SUCCESS_PATTERN   
#define DISP_NINE       0b00000001    
//extern uint8_t left_dig;
//extern uint8_t right_dig;

void set_display(uint8_t left, uint8_t right);
void display_score(uint8_t score);
void disp_init();
void swap_digit();

#endif