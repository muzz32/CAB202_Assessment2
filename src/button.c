#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t button_debounced = 0x00;

void button_init(){
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

void get_debounce(){
    static uint8_t count0 = 0;
    static uint8_t count1 = 0;

    
    uint8_t sample = PORTA.IN;
    uint8_t change = sample ^ button_debounced;
    

    count1 = (count1 ^ count0) & change;
    count0 = ~count0 & change;

    button_debounced ^= (count1 & count0);
}
