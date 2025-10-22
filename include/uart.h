#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

#define S1_1 '1'
#define S1_Q 'q'

#define S2_2 '2'
#define S2_W 'w'

#define S3_3 '3'
#define S3_E 'e'

#define S4_4 '4'
#define S4_R 'r'

#define INC_FREQ_1 ','
#define INC_FREQ_2 'k'

#define DEC_FREQ_1 '.'
#define DEC_FREQ_2 'l'

#define RESET_1 '0'
#define RESET_2 'p'

#define SEED_1 '9'
#define SEED_2 'o'

void uart_init(void);
char uart_getc(void);
void uart_putc(char c);
void uart_puts(const char* c);

//static const char valid_hex[16] = {'0','1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g'};