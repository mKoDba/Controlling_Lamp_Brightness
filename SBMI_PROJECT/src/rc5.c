/*************************************************************************
 * rc5.c
 *
 *  Created on: 13.12.2018.
 *  Authors: Mario and Nicolas Dortet Halet
 *
 *  Based on idea and code presented at:
 *  http://www.clearwater.com.au/code/rc5
 *
 *
 *  A 14-bit RC5 sequence (manchester encoded)
 *  starts with two sync bits (always 1),
 *  followed by a toggle bit (which alternates value on each key press),
 *  a 5 bit device address, and a 6-bit command number.
 *
 *  The state machine takes pulse/space events as input and generates 14-bit
 *  RC5 control codes as output.
 *  When the state machine starts it immediately emits a 1,
 *  and enters the mid1 state.
 *  Emitted bits are left-shifted into a 14-bit data store.
 *
 *  Decoding is complete when 14 bits have been emitted.
 *  It is also useful to continue decoding until the decoder is in state
 *  start1 or state mid0 , to ensure the final pulse is consumed.
 *
 *
 ************************************************************************/
#include "rc5.h"

#include <avr/io.h>
#include <avr/interrupt.h>


/* number of ticks based on manchester encoding that is used in
 * rc5 protocol */
#define LONG_MIN 2668   /* 1334 microseconds */
#define LONG_MAX 4444   /* 2222 microseconds */

typedef enum {
    STATE_START1,
    STATE_MID1,
    STATE_MID0,
    STATE_START0,
    STATE_BEGIN,
    STATE_END
} State;

/* trans[] is a table of transitions, indexed by
 * the current state.  Each byte in the table
 * represents a set of 4 possible next states,
 * packed as 4 x 2-bit values: 8 bits DDCCBBAA,
 * where AA are the low two bits, and
 *   AA = short space transition
 *   BB = short pulse transition
 *   CC = long space transition
 *   DD = long pulse transition
 *
 * If a transition does not change the state,
 * an error has occured and the state machine should
 * reset.
 *
 * The transition table is:
 * 00 00 00 01  from state 0: short space->1
 * 10 01 00 01  from state 1: short pulse->0, long pulse->2
 * 10 01 10 11  from state 2: short space->3, long space->1
 * 11 11 10 11  from state 3: short pulse->2
 */

const uint8_t trans[4] = {0x01, 0x91, 0x9b, 0xfb};
volatile uint16_t command;
uint8_t ccounter;
volatile uint8_t has_new;
State state = STATE_BEGIN;

void RC5_init(){

	EICRA |= (1<<ISC00);

    /* Reset Timer1 counter */
    TCCR1A = 0;
    /* Enable Timer1 in normal mode with TP=8 */
    TCCR1B = 0x02;

    RC5_reset();
}


void RC5_reset(){
    has_new = 0;
    ccounter = 14;
    command = 0;
    state = STATE_BEGIN;

    /* Enable INT0 */
    EIMSK |= (1<<INT0);
}


uint8_t RC5_new_command_received(uint16_t *new_command){
    if(has_new) {
        *new_command = command;
    }
    return has_new;
}

ISR(INT0_vect){
    uint16_t delay = TCNT1;
    uint8_t event;

    /* TSOP2236 pulls the data line up, giving active low,
     * so the output is inverted. If data pin is high then the edge
     * was falling and vice versa.
     *
     *  Event numbers:
     *  0 - short space
     *  2 - short pulse
     *  4 - long space
     *  6 - long pulse
     */
    if(PIND & (1<<PIND2)) event = 2;
    else event = 0;

    if(delay > LONG_MIN && delay < LONG_MAX){
        event += 4;
    }

    if(state == STATE_BEGIN){
        ccounter--;
        command |= 1 << ccounter;
        state = STATE_MID1;
        TCNT1 = 0;
        return;
    }

    State newstate = (trans[state] >> event) & 0x03;

    state = newstate;

    /* Emit 0 - just decrement bit position counter
     * cause data is already zeroed by default. */
    if(state == STATE_MID0){
        ccounter--;
    }
    else if(state == STATE_MID1){
        /* Emit 1 */
        ccounter--;
        command |= 1 << ccounter;
    }

    /* The only valid end states are MID0 and START1.
     * If we finish in MID1 we need to wait
     * for START1 so the last edge is gone. */
    if(ccounter == 0 && (state == STATE_START1 || state == STATE_MID0)){
        state = STATE_END;
        has_new = 1;
        /* Disable INT0 */
        EIMSK &= ~(1<<INT0);
    }

    TCNT1 = 0;
}
