/*
 * ra8875.c
 *
 *  Created on: Mar 2, 2021
 *      Author: cooper
 */

#include "ra8875.h"

// const vars to save on ram
const static uint16_t _width = 800;
const static uint16_t _height = 480;

/*
 * spiSend needs to send an 8 bit int over the spi lines to the ra8875
 */
void (*spiWrite)(uint8_t) = 0;
/*
 * spiRead needs to read an 8 bit integer from the desired location
 */
uint8_t (*spiRead)() = 0;

/*
 * chipSelect asserts or deasserts the spi CS line (active low)
 * based on the input. If the input is 1, the chip should be active, if 0,
 * it should be inactive
 */
void (*chipSelect)(uint8_t) = 0;

/*
 *	The reset command needs to assert the nRst pin for 1 ms, then deassert it and delay 10 ms.
 */
void (*resetRa8875)(void) = 0;

void cmdWrite(uint8_t data) {
	chipSelect(1);
	spiWrite(0x80);
	spiWrite(data);
	chipSelect(0);
}

void dataWrite(uint8_t data) {
	chipSelect(1);
	spiWrite(0x00);
	spiWrite(data);
	chipSelect(0);
}

uint8_t cmdRead() {
	chipSelect(1);
	spiWrite(0xc0);
	chipSelect(0);
	return spiRead();
}

uint8_t dataRead() {
	chipSelect(1);
	spiWrite(0x40);
	uint8_t d = spiRead();
	chipSelect(0);
	return d;
}

void setSpi(void (*write)(uint8_t), uint8_t (*read)(), void (*select)(uint8_t)) {
	spiWrite = write;
	spiRead = read;
	chipSelect = select;
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

void graphicsMode() {
	cmdWrite(0x40);
	uint8_t tmp = dataRead();

	cmdWrite(0x40);
	dataWrite((tmp & ~(0x80)));
}

void textMode() {
	cmdWrite(0x40);
	uint8_t tmp = dataRead();

	cmdWrite(0x40);
	dataWrite((tmp | 0x80));

	cmdWrite(0x21);
	tmp = dataRead();
	tmp &= ~(0xa0);
	cmdWrite(0x21);
	dataWrite(tmp);
}

void setTextColor(uint16_t c1) {
	uint8_t tmp;
	cmdWrite(0x63);
	dataWrite((c1 & 0xf800) >> 11);
	cmdWrite(0x64);
	dataWrite((c1 & 0x7e0) >> 5);
	cmdWrite(0x65);
	dataWrite((c1 & 0x001f));
	cmdWrite(0x22);
	tmp = dataRead();
	dataWrite(tmp | 0x40);
}

void setTextPosition(uint16_t x, uint16_t y) {
	cmdWrite(0x2a);
	dataWrite((x & 0xff));

	cmdWrite(0x2b);
	dataWrite((x >> 8) & 0xff);

	cmdWrite(0x2c);
	dataWrite((y & 0xff));

	cmdWrite(0x2d);
	dataWrite((y >> 8) & 0xff);
}

void screenWrite(const char *text) {
	int i = 0;
	char c = text[i];

	cmdWrite(0x02);
	while(c) {
		dataWrite(c);
		HAL_Delay(1);
		c = text[++i];
	}
}

void displayOn() {
	// first lets set it to graphics mode
	graphicsMode();

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

void enableTouch() {
	// touch panel reg
	cmdWrite(0x70);
	// enable touch, clock speed, wake enable, and clck divider
	dataWrite(0x80 | 0x70 | 0x08 | 0x04 );

	// set it to auto mode
	cmdWrite(0x71);
	// configure it to debounce
	dataWrite(0x04);

	// configures for hardware interrupt pin
	// need to preserve the state of this register though
	cmdWrite(0xf0);
	uint8_t r = dataRead();

	cmdWrite(0xf0);
	dataWrite(r | 0x04);
}

uint8_t isTouchEvent() {
	cmdWrite(0xf1);

	uint8_t d = dataRead();
	return (d & 0x04);
}

void readTouch(uint16_t *x, uint16_t *y) {

	uint16_t tmp;

	cmdWrite(0x72);
	*x = dataRead();

	cmdWrite(0x73);
	*y = dataRead();

	cmdWrite(0x74);
	tmp = dataRead();

	*x <<= 2;
	*y <<= 2;

	*x |= tmp & 0x3;
	*y |= (tmp>>2) & 0x3;

	// clear interrupt
	cmdWrite(0xf1);
	dataWrite(0x04);
}

void drawRect(uint16_t x, uint16_t y, uint16_t x2, uint16_t y2, uint16_t color) {
	// There is a built in function for making certain shapes
		// in the ra8875, this one is for a square(rectangle)

		// Configures the x parameters
		cmdWrite(0x91);
		dataWrite(x & 0xff);
		cmdWrite(0x92);
		dataWrite((x>>8) & 0xff);

		// Configures the y parameters
		cmdWrite(0x93);
		dataWrite(y & 0xff);
		cmdWrite(0x94);
		dataWrite((y >> 8) & 0xff);

		// set width
		cmdWrite(0x95);
		dataWrite(x2 & 0xff);
		cmdWrite(0x96);
		dataWrite((x2 >> 8) & 0xff);

		// set height
		cmdWrite(0x97);
		dataWrite((y2 & 0xff));
		cmdWrite(0x98);
		dataWrite((y2 >> 8) & 0xff);

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

		uint8_t wait = 0;
		do {
			cmdWrite(0x90);
			wait = dataRead();
		} while((wait & 0x80));
}
void fillScreen(uint16_t color) {
	drawRect(0,0,_width-1, _height-1, color);
}

void drawLine(uint16_t x, uint16_t y, uint16_t x1, uint16_t y1, uint16_t color) {
    cmdWrite(0x91);
    dataWrite(x);
    cmdWrite(0x92);
    dataWrite(x >> 8);

    cmdWrite(0x93);
    dataWrite(y);
    cmdWrite(0x94);
    dataWrite(y >> 8);

    cmdWrite(0x95);
    dataWrite(x1);
    cmdWrite(0x96);
    dataWrite((x1) >> 8);

    cmdWrite(0x97);
    dataWrite(y1);
    cmdWrite(0x98);
    dataWrite((y1) >> 8);


	// configure color
	cmdWrite(0x63);
	dataWrite((color & 0xf800) >> 11); // red
	cmdWrite(0x64);
	dataWrite((color & 0x07e0) >> 5); // green
	cmdWrite(0x65);
	dataWrite((color & 0x001f)); // blue

	cmdWrite(0x90);
	dataWrite(0x80);

	cmdWrite(0x90);
			uint8_t tmp = dataRead();
			while(!(tmp & 0x80)) {
				cmdWrite(0x90);
				tmp = dataRead();
			}
			HAL_Delay(20);
}
void drawPixel(uint16_t x, uint16_t y, uint16_t color) {
	cmdWrite(0x46);
	dataWrite(x & 0xff);

	cmdWrite(0x47);
	dataWrite((x>>8) & 0xff);

	cmdWrite(0x48);
	dataWrite(y & 0xff);

	cmdWrite(0x49);
	dataWrite((y>>8) & 0xff);

	cmdWrite(0x02);

	chipSelect(1);
	spiWrite(0x00);
	spiWrite((color >> 8) & 0xff);
	spiWrite(color & 0xff);
	chipSelect(0);
}
