/*
 * rc5.h
 *
 *  Created on: 19.12.2018.
 *      Author: Mario Kodba and Nicolas Dortet Halet
 */

#ifndef RC5_H
#define RC5_H

#include <stdint.h>

/* Initialize timer and interrupt */
void RC5_init();

/* Reset the library back to waiting-for-start state */
void RC5_reset();

uint8_t RC5_new_command_received(uint16_t *new_command);


#endif

