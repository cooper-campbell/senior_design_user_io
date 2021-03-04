/*
 * ra8875.c
 *
 *  Created on: Mar 2, 2021
 *      Author: cooper
 */

#include "ra8875.h"

/*
 * spiSend needs to send an 8 bit int over the spi lines to the ra8875
 */
void (*spiWrite)(uint8_t) = 0;
/*
 * spiRead needs to read an 8 bit integer from the desired location
 */
uint8_t (*spiRead)() = 0;

void cmdWrite(uint8_t data) {
	spiWrite(0x80);
	spiWrite(data);
}

void dataWrite(uint8_t data) {
	spiWrite(0x00);
	spiWrite(data);
}

uint8_t cmdRead() {
	spiWrite(0xc0);
	return spiRead();
}

uint8_t dataRead() {
	spiWrite(0x40);
	return spiRead();
}

/*
 *	The reset command needs to assert the nRst pin for 1 ms, then deassert it and delay 10 ms.
 */
void (*resetRa8875)(void) = 0;

// const vars to save on ram
const static uint16_t _width = 800;
const static uint16_t _height = 480;

void setSpi(void (*write)(uint8_t), uint8_t (*read)()) {
	spiWrite = write;
	spiRead = read;
}

void setReset(void (*resetFunc)(void)) {
	resetRa8875 = resetFunc;
}

void RA8875_PLL_init(void) {
	// this is only for 800x480 pixel display
	cmdWrite(0x88);
	dataWrite(0x0b);
	HAL_Delay(1);
	cmdWrite(0x89);
	dataWrite(0x02);
	HAL_Delay(10);
}

void LCD_Initial(void) {
	cmdWrite(0x00);
	dataRead();
	RA8875_PLL_init();
	// configure 8 bit transfers, 256 color.
	cmdWrite(0x10);
	dataWrite(0x0c);

	// more display size stuff
	cmdWrite(0x04);
	dataWrite(0x81);
	HAL_Delay(1);

	// Configure horizontal settings
	cmdWrite(0x14); // hdwr
	dataWrite(0x63); // (0x63+1)*8=800(decimal)
	cmdWrite(0x15); // hndftr
	dataWrite(0x00);
	cmdWrite(0x16); //hndr
	dataWrite(0x03);
	cmdWrite(0x17); // hstr
	dataWrite(0x03);
	cmdWrite(0x18); // hpwr
	dataWrite(0x0b);

	// vertical settings
	cmdWrite(0x19); // vdhr0
	dataWrite(0xdf);
	cmdWrite(0x1a); // vdhr1
	dataWrite(0x01);
	cmdWrite(0x1b); // vndr0
	dataWrite(0x20);
	cmdWrite(0x1c); // vndr1
	dataWrite(0x00);
	cmdWrite(0x1d); // vstr0
	dataWrite(0x16);
	cmdWrite(0x1e); // vstr1
	dataWrite(0x00);
	cmdWrite(0x1f); // vpwr
	dataWrite(0x01);

	// active window x
	cmdWrite(0x30); // hsaw0
	dataWrite(0x00);
	cmdWrite(0x31); // hsaw1
	dataWrite(0x00);
	cmdWrite(0x34); // heaw0
	dataWrite(0x1F);
	cmdWrite(0x35); // heaw1
	dataWrite(0x03);
	// active window y
	cmdWrite(0x32); // vsaw0
	dataWrite(0x00);
	cmdWrite(0x33); // vsaw1
	dataWrite(0x00);
	cmdWrite(0x36); // veaw0
	dataWrite(0xdf);
	cmdWrite(0x37); // veaw1
	dataWrite(0x01);
}

void displayOn() {
	// first lets set it to graphics mode
	cmdWrite(0x40); // mwcr0
	// there could be settings that we want to conserve here, but I am doubtful for our application.
	dataWrite(0b00000000);

	// turn on backlight
	// writeReg(RA8875_P1CR, RA8875_P1CR_ENABLE | (clock & 0xF));
	cmdWrite(0x8a);
	dataWrite(0x80 | 0x09);
	cmdWrite(0x8b);
	dataWrite(255);

	// turn display on
	cmdWrite(0x01); // pwrr
	dataWrite(0x80); // turn on

	// turn on lcd enable
	cmdWrite(0xc7); // gpiox is tied to lcd enable
	dataWrite(0x01);
}

void fillScreen(uint16_t color) {
	// There is a built in function for making certain shapes
	// in the ra8875, this one is for a square(rectangle)

	// Configures the x parameters
	cmdWrite(0x91);
	dataWrite(0x00);
	cmdWrite(0x92);
	dataWrite(0x00);

	// Configures the y parameters
	cmdWrite(0x93);
	dataWrite(0x00);
	cmdWrite(0x94);
	dataWrite(0x00);

	// set width
	cmdWrite(0x95);
	dataWrite(_width);
	cmdWrite(0x96);
	dataWrite(_width >> 8);

	// set height
	cmdWrite(0x97);
	dataWrite(_height);
	cmdWrite(0x97);
	dataWrite(_height >> 8);

	// configure color
	cmdWrite(0x63);
	dataWrite((color & 0xf800) >> 11); // red
	cmdWrite(0x64);
	dataWrite((color & 0x07e0) >> 5); // green
	cmdWrite(0x65);
	dataWrite((color & 0x001f)); // blue


	// Start the drawing function on the ra8875
	cmdWrite(0x90);
	dataWrite(0xb0);

	// TODO: need to poll the status register, for now im just gonna manually put in a bunch of delay
	HAL_Delay(1000); // 1 s delay
}
