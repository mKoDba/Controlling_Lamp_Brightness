/****************************************************************************
 * serial.c
 *  Redirection of the printf stream to the AVR serial port
 *    1. Initialize usart @ 57600 bps (change in BAUD)
 *    2. Define the low level put_char mechanism
 *    3. Redirect the printf io stream
 *
 *  Created on: 13/09/2016
 *      Author: jpsousa@fe.up.pt (eclipse + gcc-avr)
 ***************************************************************************/

#include <stdio.h>
#include <avr/io.h>          /* Register definitions*/

//#ifndef F_CPU
#define F_CPU 16000000UL               /* 16 MHz    */
//#endif
#define	BAUD 57600                     /* baud rate */
#define BAUDGEN ((F_CPU/(16*BAUD))-1)  /* divider   */

void usart_init(void) {
  UBRR0 = BAUDGEN;
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1<<RXCIE0);
  UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

int usart_putchar(char c, FILE *stream) {
  while (!( UCSR0A & (1 << UDRE0)))
    ;
  UDR0 = c;
  return 0;
}

static FILE mystdout=FDEV_SETUP_STREAM(usart_putchar,NULL,_FDEV_SETUP_WRITE);

void printf_init(void) {
  stdout = &mystdout;
}

char USARTReadChar(void){
   //Wait until a data is available

   while(!(UCSR0A & (1<<RXEN0)));

   //Now USART has got data from host
   //and is available is buffer

   return UDR0;
}


//////////////////////////////////////////////
char usart_getchar(void) {
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}
