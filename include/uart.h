#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

void uart_init(void);
char uart_getc(void);
void uart_putc(char c);
void uart_puts(const char* c);
