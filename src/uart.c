#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include "uart.h"
static int stdio_putchar(char c, FILE *stream);
static int stdio_getchar(FILE *stream);
static FILE stdio = FDEV_SETUP_STREAM(stdio_putchar, stdio_getchar, _FDEV_SETUP_RW);

void uart_init(void)
{
    PORTB.DIRSET = PIN2_bm;
    USART0.BAUD = 1389;                           // 9600 baud @ 3.333 MHz
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm; // Enable Tx/Rx
    stdout = &stdio;
    stdin = &stdio;
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