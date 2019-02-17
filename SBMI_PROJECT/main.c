/*************************************************************************
 *
 *
 *  Created on: 18.11.2018.
 *  Authors: Mario Kodba and Nicolas Dortet-Halet
 *
 *  The project was done as part of practical class of Sistemas Baseados
 *	em Microprocessadores (SBMI) - Microprocessor based Systems and Interfaces at FEUP.
 *	All the working materials, such as the lamp,AC to DC converting
 *	circuit, Arduino was provided by the faculty.
 *
 *	The goal was to design system that controls a lamp in synchronism
 *	with the voltage of the 230V electrical network, using a push-button.
 *	Each press of the push-button should increase the brightness of the
 *	lamp by 25%. That is, in the sequence 0-25-50-75-100-0-25 ....
 *	The push button (PD4) is an active low component and each press
 *	is detected in the main while loop,after which is duty cycle increased
 *
 *	Zero Cross Detection (ZCD) wire is connected to atmega328p as input
 *	at PD3 which serves as an external interrupt pin 1 (INT1).
 *	Upon detecting the zero cross of a sine wave, the lamp is turned off.
 *	At the same time, we put value stored in variable 'duty' into timer 2,
 *	which is working in normal mode, with a prescaler of 1024.
 *	Timer 1 is used for RC5 decoding since it needs to count higher values
 *
 *	Since power of the lamp is not linear function, value of duty cycle
 *	was calculated while taking into consideration
 *	area under sin^2(t) function.
 *	At timer2 overflow lamp turns on and waits next falling edge of
 *	ZCD signal after which it turns off again and the program repeats.
 *
 *
 *	It is also possible to control the lamp in the above sequence
 *	by pressing 'space' key on keyboard which is received through serial
 *  port of atmega328p as an USART_RX interrupt.
 *
 *  Additionally, IR receiver (TSOP2236) was connected to atmega328p.
 *  It uses standard RC5 IR coding protocol which is explained in rc5.c.
 *  By receiving certain command from IR remote (in this case command with
 *  a number 16) it will increase brightness in the same sequence as
 *  described above.
 *
 *  By defining DEBUG macro at the beginning of the program,it is possible
 *  to print output to USART port for debugging and program control.
 *
 ************************************************************************/


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "serial.h"
#include "rc5.h"

#define DEBUG

/* Defining number of ticks for each duty cycle needed.
 * Since 230V grid is 50Hz, T=20ms and each half period of the sine wave
 * is 10ms long.
 */

#define BUTTON PD4

#define ZCD PD3			/* Interrupt pin INT1 */
#define CTRL PB0		/* Lamp turn on/off control pin */

#define LAMP_OFF 	PORTB&=~(1<<CTRL)
#define LAMP_ON 	PORTB|=(1<<CTRL)
#define MAX 255
#define RC5_get_command_bits(command) (command & 0x3F)

volatile uint8_t aux=1;
volatile uint16_t n=0, duty;

void tc2_init() {

	TCCR2B = 0;			/* Stop timer2 and clear pending instructions */
	TIFR2 |= (7<<TOV2);

	TCCR2A = 0;			/* NORMAL mode and enable overflow interrupt */
	TIMSK2 |= (1<<TOIE2);
}

void hw_init(){

	/*CTRL output for turning on/off lamp*/
	DDRB = DDRB | (1<<CTRL);

	/* Set BUTTON and ZCD pin as input
	   and activate its internal pull-up */
	DDRD &= ~(1<<BUTTON);
	PORTD |= (1<<BUTTON);

	DDRD &= ~(1<<ZCD);
	PORTD |= (1<<ZCD);


	/* Interrupt request at FALLING edge
	* for INT1 - zcd signal */
	EICRA |= (2<<ISC10);
	/* Enable INT1 */
	EIMSK |= (1<<INT1);


	/* Enable global interrupt flag */
	sei();
	usart_init();

	#ifdef DEBUG
	printf_init();

	printf("\n\nHello\n");
	#endif
}

int main(void){

	hw_init();
	tc2_init();

	RC5_init();


    while(1){

		#ifdef DEBUG
    	if(aux!=n) {
    		printf("%d ", n);
    		aux=n;
    	}
		#endif

    	if(!(PIND & (1<<BUTTON))){
    		n++;
    		if(0==n) duty = 154;
    		if(1==n) duty = 104;
    		if(2==n) duty = 78;
    		if(3==n) duty = 52;
    		if(4==n) duty = 1;
    		if(5==n){
    			n=0;
    			duty=154;
    		}
    		printf("\npressed - %d\n", n);
    	}
    	while(!(PIND & (1<<BUTTON)));

    	uint16_t command,cmd;

    	if(RC5_new_command_received(&command)){
    		printf("New command received!");
    	    /* Reset RC5 so the next command can be decoded */
    	    RC5_reset();
    	    cmd = RC5_get_command_bits(command);
    	    printf("\nCommand bits received:  %d\n", cmd);
    	    if(16 == cmd){
    	    n++;
    	    	if(0==n) duty = 154;
    	    	if(1==n) duty = 104;
    	    	if(2==n) duty = 78;
    	    	if(3==n) duty = 52;
    	    	if(4==n) duty = 1;
    	    	if(5==n){
    	    		n=0;
    	    		duty=154;
    	    	}
    	    }
    	}

    }
    return 0;
}

ISR(USART_RX_vect){
	volatile uint8_t c;
	    	c = usart_getchar();
	    	printf("Received input: %d\n", c);
	    	/* If 'space' key pressed change brightness of the lamp */
	    	if(c == 32){
	    		n++;
	    		if(0==n) duty = 154;
	    		if(1==n) duty = 104;
	    		if(2==n) duty = 78;
	    		if(3==n) duty = 52;
	    		if(4==n) duty = 1;
	    		if(5==n){
	    			n=0;
	    			duty=154;
	    		}
	    	}

}

/*ISR for ZCD signal*/
ISR(INT1_vect){

	LAMP_OFF;
	TCNT2 = MAX - duty;		/* Put the value in timer, depending on
							   the brightness we want */
	TCCR2B = 0x07;			/* Start timer2 with TP=1024 */
}

/*TIMER2 ISR */
ISR(TIMER2_OVF_vect){

	LAMP_ON;			// Upon reaching overflow, turn the lamp on
}

