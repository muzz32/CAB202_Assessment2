#include <stdint.h>
#define disp_lhs        0b10000000
#define disp_off        0b01111111
#define disp_bar_1      0b00111110
#define disp_bar_2      0b01101011 
#define success_pattern 0b00000000
#define fail_pattern    0b01110111
volatile uint8_t left_dig;
volatile uint8_t right_dig;