#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t button_debounced; //A bit mask that represents the debounced state of the buttons

/*
Initialises PORTA to be able to read inputs from the 
push buttons on the QUTY by enabling pullup resistors
*/
void button_init(){
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm; // Enable pull up resistors for buttons S1-S4
    button_debounced = 0x00; // Set debounced state to 0 initially (no button press)
}

/*
Uses a vertical counter to detect a button input only if its detected 3
consecutive times (Used in 5ms interrupt, so gets button states every 15ms)
*/
void get_debounce(){
    static uint8_t count0 = 0;
    static uint8_t count1 = 0; // Two counters creates a 2 bit counter (up to 3)

    
    uint8_t sample = PORTA.IN; // Get current state of PORTA
    uint8_t change = sample ^ button_debounced; //Get whats changed by comparing it to current buttonstate
    
    // Intrement counter if change, otherwise both get reset to 0
    count1 = (count1 ^ count0) & change; 
    count0 = ~count0 & change;

    // Get the current debounced state of PORTA buttons
    button_debounced ^= (count1 & count0);
}
