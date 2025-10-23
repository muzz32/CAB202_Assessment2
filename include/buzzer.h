#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void set_buzzer(uint8_t index);
void buzzer_off();
void increase_octave();
void decrease_octave();
void buzzer_init();

#endif