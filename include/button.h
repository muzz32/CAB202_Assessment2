#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

volatile uint8_t button_debounced;
void button_init();
void get_debounce();

#endif