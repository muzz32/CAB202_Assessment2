#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "uart.h"
#include "buzzer.h"
#include "game.h"
#include "sequence.h"
static int stdio_putchar(char c, FILE *stream);
static int stdio_getchar(FILE *stream);
static FILE stdio = FDEV_SETUP_STREAM(stdio_putchar, stdio_getchar, _FDEV_SETUP_RW);
volatile uint8_t uart_input_recieved;
volatile uint8_t uart_input;
uint8_t getting_seed;
uint8_t seed_index;
volatile uint8_t seed_ready;
uint8_t get_seed(uint8_t seed_index, char char_input);
volatile char hex_seed[9]; 
void uart_init(void)
{
    PORTB.DIRSET = PIN2_bm;
    USART0.BAUD = 1389;                           // 9600 baud @ 3.333 MHz
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm; // Enable Tx/Rx
    USART0.CTRLA = USART_RXCIE_bm; // Enable receive complete interrupt
    stdout = &stdio;
    stdin = &stdio;
    uart_input_recieved = 0;
    uart_input = -1; //Initialised but not a valid value to avoid 
    getting_seed = 0;
    seed_index = 0;
    seed_ready = 0;
}

char uart_getc(void)
{
    while (!(USART0.STATUS & USART_RXCIF_bm));
    return USART0.RXDATAL;
}

void uart_putc(char c)
{
    while (!(USART0.STATUS & USART_DREIF_bm)); // Wait for TXDATA empty
    USART0.TXDATAL = c;
}


void uart_puts(const char* c){
    while (*c != '\0')
    {
        uart_putc(*c); //Put the value at c
        c++; //Increment c
    }
}

static int stdio_putchar(char c, FILE *stream) {
    uart_putc(c);
    return c; 
}
static int stdio_getchar(FILE *stream) {
    return uart_getc();
}

uint8_t get_seed(uint8_t seed_index, char char_input){
    uint8_t temp_valid = 0;
    static uint8_t valid;
    if((char_input >= '0' && char_input <= '9') || (char_input >= 'a' && char_input <= 'f')){
        temp_valid = 1;
    }
    else{
        temp_valid = 0;
    }

    if(!seed_index && temp_valid){
        valid = 1;
    }
    else{
        valid &= temp_valid;
    }

    hex_seed[seed_index] = char_input;
    printf("%c\n", char_input);
    return valid;
}

ISR(USART0_RXC_vect){

    uint8_t char_recieved = USART0.RXDATAL;

    if(getting_seed){
        if (seed_index < 7)
        {
            get_seed(seed_index, char_recieved);
            seed_index++;
        }
        else if (seed_index == 7){
            uint8_t seed_status = get_seed(seed_index, char_recieved);
            if (seed_status){
                hex_seed[8]= '\0';
                seed_ready = 1;
                printf("seeddone");
                pre_seed_state = state;
                state = SEED;
            }
            else{
                seed_ready = 0;
            }
            getting_seed = 0;
            seed_index = 0;
        }
    }
    else{
        switch (char_recieved)
        {
            case S1_1:
            case S1_Q:
                uart_input = 0;
                uart_input_recieved = 1;
                //printf("1");
                break;
            case S2_2:
            case S2_W:
                uart_input = 1;
                uart_input_recieved = 1;
                //printf("2");
                break;
            case S3_3:
            case S3_E:
                uart_input = 2;
                uart_input_recieved = 1;
                //printf("3");
                break;
            case S4_4:
            case S4_R:
                uart_input = 3;
                uart_input_recieved = 1;
                //printf("4");
                break;
            case INC_FREQ_1:
            case INC_FREQ_2:
                increase_octave();
                //printf("inc");
                break;
            case DEC_FREQ_1:
            case DEC_FREQ_2:
                decrease_octave();
                //printf("dec");
                break;
            case RESET_1:
            case RESET_2:
                state = RESET;
                //printf("res");
                break;
            case SEED_1:
            case SEED_2:
                getting_seed = 1;
                //printf("gettingseed\n");
                break;
            default:
                break;
        }
    }
}