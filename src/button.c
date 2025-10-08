#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t pb_debounced_state = 0xFF;

void pushbut_init(){
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

void get_debounce(){
    static uint8_t count0 = 0;
    static uint8_t count1 = 0;

    
    uint8_t pb_sample = PORTA.IN;
    uint8_t change = pb_sample ^ pb_debounced_state;
    

    count1 = (count1 ^ count0) & change;
    count0 = ~count0 & change;

    pb_debounced_state ^= (count1 & count0);
}
