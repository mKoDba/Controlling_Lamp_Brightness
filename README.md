# Microprocessor Based Systems and Interfaces - Sistemas Baseados em Microprocessadores (SBMI)

Project done at Porto University as part of 'Microprocessor Based Systems and Interfaces' course

Authors: Mario Kodba and Nicolas Dortet-Halet
 
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
