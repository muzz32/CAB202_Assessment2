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



ISR(USART0_RXC_vect){
    static uint8_t getting_seed = 0;
    static uint8_t seed_index = 0;

    uint8_t char_recieved = USART0.RXDATAL;

    if(getting_seed){
        static char hex_string[8];
        static uint8_t valid = 0;
        if (seed_index == 8)
        {
            if(valid){
                state = SEED;
            } 
            getting_seed = 0;
            seed_index = 0;
            valid = 0;
        } 
        else{
            uint8_t temp_valid = 0;
            for(uint8_t i = 0; i < 15; i++){
                if (char_recieved == valid_hex[i])
                {
                    temp_valid = 1;
                }
            }
            if(temp_valid){
                if((!valid && !seed_index) || (valid && seed_index)){ // Add only if the 
                    valid = 1;
                    hex_seed[seed_index] = char_recieved;
                }
            }
            else{
                valid = 0;
            }
            seed_index++;
        }
    }
    else{
        switch (char_recieved)
        {
            case S1_1:
            case S1_Q:
                uart_input = 0;
                uart_input_recieved = 1;
                printf("1");
                break;
            case S2_2:
            case S2_W:
                uart_input = 1;
                uart_input_recieved = 1;
                printf("2");
                break;
            case S3_3:
            case S3_E:
                uart_input = 2;
                uart_input_recieved = 1;
                printf("3");
                break;
            case S4_4:
            case S4_R:
                uart_input = 3;
                uart_input_recieved = 1;
                printf("4");
                break;
            case INC_FREQ_1:
            case INC_FREQ_2:
                increase_octave();
                printf("inc");
                break;
            case DEC_FREQ_1:
            case DEC_FREQ_2:
                decrease_octave();
                printf("dec");
                break;
            case RESET_1:
            case RESET_2:
                state = RESET;
                printf("res");
                break;
            case SEED_1:
            case SEED_2:
                getting_seed = 1;
                printf("sed");
            default:
                break;
        }
    }
}