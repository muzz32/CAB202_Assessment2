#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>
void adc_init();
void update_delay();
extern uint16_t playback_delay; //External so the game loop can access it

#endif