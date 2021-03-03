/*
 * ra8875.h
 *
 *  Created on: Mar 2, 2021
 *      Author: cooper
 */

#ifndef INC_RA8875_H_
#define INC_RA8875_H_

// this is all derived from https://cdn-shop.adafruit.com/datasheets/ra8875+app+note.pdf

#include <stdint.h>
#include "stm32f0xx_hal.h"

#define RA8875_DATAWRITE 0x00
#define RA8875_DATAREAD 0x40
#define RA8875_CMDWRITE 0x80
#define RA8875_CMDREAD 0xC0

/*
 * The spi write command needs to send an 8 bit integer to the ra8875 screen
 * The spi read command needs to read an 8 bit integer to the ra8875 screen
 */
void setSpi(void (*write)(uint8_t), uint8_t (*read)());

/*
 *	The reset command needs to assert the nRst pin for 1 ms, then deassert it and delay 10 ms.
*/
void setReset(void (*resetFunc)(void));

// This is primarily for internal use
void RA8875_PLL_init(void);
// This is the first thing that should be called after a reset.
void LCD_Initial(void);

// This will configure the display on for our purposes
void displayOn();

// makes a screen one color
void fillScreen(uint16_t c);

#endif /* INC_RA8875_H_ */
