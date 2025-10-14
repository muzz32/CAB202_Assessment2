#include <stdint.h>
#define DISP_LHS        0b10000000
#define DISP_OFF        0b01111111
#define DISP_BAR_1      0b00111110
#define DISP_BAR_2      0b01101011 
#define SUCCESS_PATTERN 0b00000000
#define FAIL_PATTERN    0b01110111
//extern uint8_t left_dig;
//extern uint8_t right_dig;

void set_display(uint8_t left, uint8_t right);
void disp_init();
void swap_digit();