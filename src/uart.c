#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "uart.h"
#include "buzzer.h"
#include "game.h"
#include "sequence.h"

// Prototypes needed for FDEV setup
static int stdio_putchar(char c, FILE *stream);
static int stdio_getchar(FILE *stream);

// Sets the stdio to use USART for read and write, allowing for use of printf
// and scanf
static FILE stdio = FDEV_SETUP_STREAM(stdio_putchar, stdio_getchar, _FDEV_SETUP_RW);

volatile uint8_t uart_input_recieved;
volatile uint8_t uart_input;
uint8_t getting_seed;
uint8_t seed_index;
volatile uint8_t seed_ready;
uint8_t get_seed(uint8_t seed_index, char char_input);
volatile char hex_seed[9]; 
volatile uint8_t name_index;
volatile char temp_name[21];
volatile uint8_t name_ready;
volatile uint8_t name_input_received;

/*
Initialises the USART peripheral to read and write to stdout/stdin.
Also sets any relavant variables
*/
void uart_init(void)
{
    PORTB.DIRSET = PIN2_bm; //Set the Tx pin as output
    USART0.BAUD = 1389; // 9600 baud @ 3.333 MHz
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm; // Enable Tx/Rx
    USART0.CTRLA = USART_RXCIE_bm; // Enable receive complete interrupt
    stdout = &stdio; 
    stdin = &stdio; //Sets stdout and stdin to use the UART stream
    uart_input_recieved = 0;
    uart_input = -1; //Initialised but not a valid value to avoid 
    getting_seed = 0;
    seed_index = 0;
    seed_ready = 0;
    name_index = 0;
    name_ready = 0;
    name_input_received = 0;
}

/*
Waits until a char has been recieved over UART, and then returns it
*/
char uart_getc(void)
{
    while (!(USART0.STATUS & USART_RXCIF_bm));
    return USART0.RXDATAL;
}

/*
Waits until previous data has been transmitted, and then writes
char c to the TX register to be transmitted over UART
*/
void uart_putc(char c)
{
    while (!(USART0.STATUS & USART_DREIF_bm)); // Wait for TXDATA empty
    USART0.TXDATAL = c;
}


/*
This function is called by standard C functions such as printf, and means it will
use the uart_putc function to write to stdout
*/
static int stdio_putchar(char c, FILE *stream) {
    uart_putc(c);
    return c; 
}

/*
This function is called by standard C functions such as scanf, and means it will
use the uart_getc function to read from stdin
*/
static int stdio_getchar(FILE *stream) {
    return uart_getc();
}

/*
Used to print out the highscore_table. First prints a new line, then for each
non empty_user entry in the list it'll print the users name and score, along 
with a new line. When an empty_user entry is found, stop printing and exit the
function.
*/
void print_user_table(USER *table, uint8_t table_length){
    printf("\n");
    for (uint8_t i = 0; i < table_length; i++)
    {
        if (table[i].score == 0) break;
        printf("%s %u\n", table[i].name, table[i].score);
    }
    
}

/*
Used to add chars to the hex_seed array, and validate whether or not its a 
valid hex literal. Returns a zero if the hex_seed contains a non hex character,
returns a 1 otherwise.
*/
uint8_t get_seed(uint8_t seed_index, char char_input){
    uint8_t temp_valid = 0;
    static uint8_t seed_valid; 

    // If the charachter is a number or a letter between a and f, its valid. other wise its not
    if((char_input >= '0' && char_input <= '9') || (char_input >= 'a' && char_input <= 'f')){
        temp_valid = 1;
    }
    else{
        temp_valid = 0;
    }

    // If this is the first entry in the array and its valid, set seed_valid to 1
    if(!seed_index && temp_valid){
        seed_valid = 1;
    }
    // Otherwise, get the hex_seeds validity by logic anding the current seed_valid with temp_valid.
    // This will clear it if an invalid character is added to hex_seed, and not set it to one if a valid
    // Character is added after
    else{
        seed_valid = seed_valid && temp_valid;
    }
    hex_seed[seed_index] = char_input; // Always add the char to the array
    return seed_valid;
}


/*
This interrupt triggers everytime a character is recieved over UART
*/
ISR(USART0_RXC_vect){
    char char_recieved = USART0.RXDATAL;
    /*
    If the state is GET_HIGHSCORE, all characters should be assumed to be name inputs.
    If the char entered is a new line, the name is done, so null index it and signal its ready.
    Otherwise, if the name_index is less than 19 (means less than 20 chars), add the 
    character to the temp_name array and increase the index to accomodate the next entry. 
    If its now equal to 19, the name is full, so null index the string. No more inputs will 
    be added to the string.
    */
    if (state == GET_HIGHSCORE){
        if(char_recieved == '\n'){
            temp_name[name_index] = '\0';
            name_ready = 1;
        }
        else if(name_index < 19){
            temp_name[name_index] = char_recieved;
            name_index++;
            if(name_index == 19){
                temp_name[20] = '\0';
            }
        }
        name_input_received = 1;
    }
    /*
    If no name is being entered, and a SEED command was triggered by the user, they can continue 
    adding to the seed.

    If the seed index is less than 7 (less than 8 char entered), add the char to the hex_seed array
    with get_seed and increment the seed_index.

    If the seed index is 7 add the last char and check the whole arrays validity. If it is a valid 
    hex number, use strtoul, which converts a string to a unsigned long hex integer (uint32_t), to update
    the seq_seed variable, and signal that the seed had been updated by setting seed_ready. If its not valid,
    terminate the array and clear the seed_ready variable. 
    Also clear the getting_seed and seed_index to go back to regular UART inputs and prepare for new seed entries
    */
    else if(getting_seed){
        if (seed_index < 7)
        {
            get_seed(seed_index, char_recieved);
            seed_index++;
        }
        else if (seed_index == 7){
            uint8_t seed_status = get_seed(seed_index, char_recieved);
            if (seed_status){
                hex_seed[8]= '\0';
                seq_seed = strtoul((const char*)hex_seed, NULL, 16);
                seed_ready = 1;
            }
            else{
                hex_seed[0]= '\0';
                seed_ready = 0;
            }
            getting_seed = 0;
            seed_index = 0;
        }
    }
    /*
    If the above two cases aren't met, check if the input matches one of the predefined
    chars in uart.h, and carry out the relevant code.
    */
    else{
        switch (char_recieved)
        {   /*
            If the input matches one of the push button chars, set the uart_input
            to what it is and set the uart_input_recieved variable to signal that
            the game should use this input and not wait for a button press.
            */
            case S1_1:
            case S1_Q:
                uart_input = 0;
                uart_input_recieved = 1;
                //printf("1/q");
                break;
            case S2_2:
            case S2_W:
                uart_input = 1;
                uart_input_recieved = 1;
                //printf("2/w");
                break;
            case S3_3:
            case S3_E:
                uart_input = 2;
                uart_input_recieved = 1;
                //printf("3/e");
                break;
            case S4_4:
            case S4_R:
                uart_input = 3;
                uart_input_recieved = 1;
                //printf("4/r");
                break;
            /*
            If the input is the char for Increasing or decreasing the octave, run 
            that function
            */
            case INC_FREQ_1:
            case INC_FREQ_2:
                increase_octave();
                break;
            case DEC_FREQ_1:
            case DEC_FREQ_2:
                decrease_octave();
                break;
            /*
            If the input is the char for reseting the game, set the game state to 
            RESET
            */
            case RESET_1:
            case RESET_2:
                state = RESET;
                break;
            /*
            If the input is the char for changing the seed, set tgetting_seed variable to start 
            getting seed inputs
            */
            case SEED_1:
            case SEED_2:
                getting_seed = 1;
                seed_index = 0;
                break;
            default:
                break;
        }
    }

}