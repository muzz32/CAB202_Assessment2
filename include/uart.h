#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include "game.h"

/*
Characters that are used for triggering different game events over UART
*/
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
void print_user_table(USER *table, uint8_t table_length);


extern volatile uint8_t uart_input_recieved;
extern volatile uint8_t uart_input;
extern volatile uint8_t seed_ready;
extern volatile char hex_seed[9]; 
extern volatile char temp_name[21];
extern volatile uint8_t name_ready;
extern volatile uint8_t name_input_received;
extern volatile uint8_t name_index;

#endif