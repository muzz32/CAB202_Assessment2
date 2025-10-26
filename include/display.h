#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>


/*
Below outlines some bitmasks that are used to set different digits of the display
the msb of the bit masks determines which side it turns on. Setting it puts it on
the left hand side (as seen at DISP_LHS) and clearing it puts it on the right hand 
side. All digits are by default put on the right, and by or-ing the masks with the
LHS mask, that position can be changed.
*/
//     _____
//    |  A  |
//   F|     |B
//    |_____|
//    |  G  |
//   E|     |C
//    |_____|   
//       D  
                        //xFABGCDE
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


void set_display(uint8_t left, uint8_t right);
void display_score(uint16_t score);
void disp_init();
void swap_digit();

#endif