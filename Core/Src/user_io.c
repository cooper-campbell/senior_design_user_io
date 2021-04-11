/*
 * user_io.c
 *
 *  Created on: Feb 14, 2021
 *      Author: cooper
 *  Description: This file aims to have all the necessary code for
 *  the User IO subsystem.
 */

#include "user_io.h"
#include <stdint.h>

#define USR_WRTN_TAG 0x4000
#define STARTY 100
// Private vars
struct {
	int16_t waveform_s_t[WAVEFORM_SIZE];
	union {
		struct {
			uint8_t attack;
			uint8_t decay;
		};
		uint16_t word_1;
	};
	union {
		struct {
			uint8_t sustain;
			uint8_t release;
		};
		uint16_t word_2;
	};
} data_send_struct;

int16_t *waveform = (data_send_struct.waveform_s_t);
// anonymous structure to count waveform properly
// almost out of ram already so time for bitfields
I2C_HandleTypeDef storage_i2c;
SPI_HandleTypeDef m4_spi;
SPI_HandleTypeDef screen_spi;
//DMA_HandleTypeDef m4_dma;

uint16_t scalex = 30;
uint16_t scaley = 130;
uint16_t divx = 950;
uint16_t divy = 818;

volatile uint8_t button_press = 0;
volatile uint8_t finish_spi_send = 0;

int least_written = 800;
int most_written = -1;

uint8_t unsaved_changes = 0;

static const uint16_t width = 50;
static const uint16_t margin = 100;
static const uint8_t border = 2;

enum menu_options {
	SAVE,
	LOAD,
	SEND,
	ASDR,
	CALIBRATE,
	CLEAR,
	BACK
};

// functions for the ra8875 to use
void resetChip() {
	// the time for this is exaggerated to make sure that it works
	HAL_GPIO_WritePin(LcdSyncReset_GPIO_Port, LcdSyncReset_Pin, GPIO_PIN_RESET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(LcdSyncReset_GPIO_Port, LcdSyncReset_Pin, GPIO_PIN_SET);
	HAL_Delay(3000);
}

void selectChip(uint8_t on) {
	if(on) HAL_GPIO_WritePin(ScreenSelect_GPIO_Port, ScreenSelect_Pin, GPIO_PIN_RESET);
	else HAL_GPIO_WritePin(ScreenSelect_GPIO_Port, ScreenSelect_Pin, GPIO_PIN_SET);
}

void spiSend(uint8_t d) {
	//HAL_GPIO_WritePin(ScreenSelect_GPIO_Port, ScreenSelect_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&screen_spi, &d, 1, HAL_MAX_DELAY);
	//HAL_GPIO_WritePin(ScreenSelect_GPIO_Port, ScreenSelect_Pin, GPIO_PIN_SET);
}

uint8_t spiReceive() {
	uint8_t d = 0;
	//HAL_GPIO_WritePin(ScreenSelect_GPIO_Port, ScreenSelect_Pin, GPIO_PIN_RESET);
	HAL_SPI_Receive(&screen_spi, &d, 1, HAL_MAX_DELAY);
	//HAL_GPIO_WritePin(ScreenSelect_GPIO_Port, ScreenSelect_Pin, GPIO_PIN_SET);
	return d;
}
// Maps 0,799 to the location in waveform of the sample.
int find_location(int x) {
	int tmp = 2 * (x+1);
	return tmp + tmp/10 - tmp/100 -1;
}

int16_t scale(uint16_t y) {
	int tmp = y - 240;
	return 136 * tmp + tmp/2;
}

uint16_t undoScale(int16_t y) {
	return (y/136.5) + 240;
}

void transmit_wavetable() {
	HAL_GPIO_WritePin(SPI_NSS_GPIO_Port, SPI_NSS_Pin, RESET);
	HAL_SPI_Transmit(&m4_spi, (uint8_t *)waveform, WAVEFORM_SIZE, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&m4_spi, (uint8_t *)&data_send_struct.word_1, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&m4_spi, (uint8_t *)&data_send_struct.word_2, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(SPI_NSS_GPIO_Port, SPI_NSS_Pin, SET);
}

void reset_waveform() {
	for(int i = 0; i < WAVEFORM_SIZE; i++) {
		waveform[i] = 0;
	}
}

void stream_touch_sample(uint16_t x, uint16_t y) {
	int location = find_location(x);
	waveform[location] = USR_WRTN_TAG | y;
	if(x > most_written) most_written = x;
	if(x < least_written) least_written = x;
}

void interpolate_user_streamed_wave() {
	int16_t copy = waveform[find_location(least_written)];

	for(int i = 0; i < least_written; i++) {
		waveform[find_location(i)] = copy;
	}
	for(int i = least_written+1; i < 800; i++) {
		int16_t tmp = waveform[find_location(i)];
		if(tmp & USR_WRTN_TAG) copy = tmp;
		else waveform[find_location(i)] = copy;
	}
}

void scale_user_streamed() {
	for(int i = 0; i < 800; i++) {
		int16_t val = waveform[find_location(i)];
		waveform[find_location(i)] = scale(val & (~USR_WRTN_TAG));
	}
}

void interpolate_waveform() {
	// So we double every sample, by adding 1 sample before it
	// Next, every 10 samples we add a new one, skipping every 100 samples
	int16_t value_before = waveform[WAVEFORM_SIZE-1];
	int16_t value_after;

	// It is easier to loop over every point given and interpolate between
	int alocation = find_location(0);
	int blocation = find_location(799);
	int num = 2;
	value_before = waveform[blocation];
	value_after = waveform[alocation];
	int index = 0;
	for(int i = 0; i < 800; i++) {
		for(int j = 1; j < num; j++) {
			waveform[index] = (j * (value_after - value_before))/num + value_before;
			index++;
		}
		index++;
		blocation = alocation;
		alocation = find_location((i+1)%800);
		num = alocation - blocation;
		value_before = waveform[blocation];
		value_after = waveform[alocation];
	}
	value_after = waveform[find_location(0)];
	value_before = waveform[find_location(799)];
	// add final two points to smooth towards the beginning.
	waveform[WAVEFORM_SIZE-2] = (value_after - value_before) / 3 + value_before;
	waveform[WAVEFORM_SIZE-1] = (2*(value_after - value_before)) / 3 + value_before;
	waveform[0] = waveform[WAVEFORM_SIZE-1];
}

void normalize_waveform() {
	int sum = 0;
	for(int i = 0; i < WAVEFORM_SIZE; i++) {
		sum += waveform[i];
	}
	// Now we start removing DC offset as best we can.
	// also I know this has potential for weird maths bc of the unsigned/signed discrepancy.
	int16_t average_whole = sum/WAVEFORM_SIZE;
	//int16_t average_remainder = sum % WAVEFORM_SIZE;
	// Remove the whole number offset from the waveform.
	for(int i = 0; i < WAVEFORM_SIZE; i++) {
		waveform[i] = waveform[i] - average_whole;
	}
	/*
	// just return early if there is no remainder (unlikely).
	if(average_remainder == 0) return;
	// Do our best to remove fractional part
	// I may have to get more precise by finding the remainder of the remainder division even.
	multiplier = 1;
	for(uint16_t i = 0; i < WAVEFORM_SIZE; i++) {
		if(i % average_remainder == 0) {
			if(waveform[i] >= 1) {
				waveform[i] = waveform[i] - multiplier * ((average_whole < 0) ? -1 : 1);
				multiplier = 1;
			}
			else {
				multiplier++;
			}
		}
	}
	*/
}

void wait_user_touch() {
	while(!isTouchEvent());
	uint16_t tmp;

	while(isTouchEvent()) {
		readTouch(&tmp, &tmp);
	}
	HAL_Delay(250);
	while(isTouchEvent()) {
		readTouch(&tmp, &tmp);
	}
}

void calibrate() {
	uint16_t rectx1, recty1, rectx2, recty2, rectx3, recty3, rectx4, recty4;

	drawRect(0, 0, 4, 4, 0xf800);
	while(!isTouchEvent());
	HAL_Delay(500);
	readTouch(&rectx1, &recty1);
	drawRect(0, 0, 4, 4, 0xffff);
	HAL_Delay(1000);

	drawRect(795, 0, 799, 4, 0xf800);
	while(!isTouchEvent());
	HAL_Delay(500);
	readTouch(&rectx2, &recty2);
	drawRect(795, 0, 799, 4, 0xffff);
	HAL_Delay(1000);

	drawRect(795, 475, 799, 479, 0xf800);
	while(!isTouchEvent());
	HAL_Delay(500);
	readTouch(&rectx3, &recty3);
	drawRect(795, 475, 799, 479, 0xffff);
	HAL_Delay(1000);

	drawRect(0, 475, 4, 479, 0xf800);
	while(!isTouchEvent());
	HAL_Delay(500);
	readTouch(&rectx4, &recty4);
	drawRect(0, 475, 4, 479, 0xffff);
	HAL_Delay(1000);

	if(rectx1 < rectx4) scalex = rectx1;
	else scalex = rectx4;
	if(rectx2 > rectx3) divx = rectx2-scalex;
	else divx = rectx3 - scalex;

	if(recty1 < recty4) scaley = recty1;
	else scaley = recty4;
	if(recty2 > recty3) divy = recty2-scaley;
	else divy = recty3 - scaley;
}

uint8_t restoreCalibration() {
	uint8_t receive_buffer[1] = {0};
	uint16_t set_address = CALIBRATION_DATA_ID;

	HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)&set_address, 2, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)receive_buffer, 1, HAL_MAX_DELAY);
	if(receive_buffer[0] != 0xaa)
		return 0;

	set_address = CALIBRATION_DATA_START;
	HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)&set_address, 2, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)&scalex, 2, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)&scaley, 2, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)&divx, 2, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)&divy, 2, HAL_MAX_DELAY);
	return 1;
}

void wait_user_input() {
	uint16_t tmp;
	while(1) {
		if(isTouchEvent()) {
			readTouch(&tmp, &tmp);
			break;
		}
		if(button_press) {
			button_press = 0;
			break;
		}
	}
}
// eeprom has different byte ordering.
uint16_t byte_flip(uint16_t address) {
	uint16_t tmp = (address >> 8) & 0x0f;
	tmp |= ((address & 0xff) << 8);
	return tmp;
}

void set_send_wavetable_prorgess(uint16_t num) {

	uint16_t px_to_draw = num *800 / WAVEFORM_SIZE;
	drawRect(0,121, px_to_draw, 179, 0x001f);
}

uint8_t sendWavetableI2C() {
	// eeprom supports 32 bit writes
	// need 16 bits for the 12 address bits
	uint16_t send_buffer[1 + 2] = {byte_flip(WAVEFORM_DATA_START), 0, 0};

	// send wavetable
	for(int i = 0; i < WAVEFORM_SIZE; i+=2) {
		// update the write address
		send_buffer[0] = byte_flip(WAVEFORM_DATA_START + i * 2);

		// move over waveform information to the buffer
		send_buffer[1] = waveform[i];
		send_buffer[2] = waveform[i+1];

		int x = HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)send_buffer, 6, HAL_MAX_DELAY);
		if(x == HAL_ERROR) {
			return 0;
		}
		set_send_wavetable_prorgess(i);
		HAL_Delay(3);
	}
	// set confirmation of wavetable
	send_buffer[0] = byte_flip(WAVEFORM_ID_ADDR);
	send_buffer[1] = 0xaaaa;
	HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)send_buffer, 3, HAL_MAX_DELAY);
	return 1;
}

uint8_t restoreWaveTableI2C() {
	// check if there is a wavetable
	uint8_t receive_buffer[1] = {0};
	uint16_t set_address = byte_flip(WAVEFORM_ID_ADDR);
	HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)&set_address, 2, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)receive_buffer, 1, HAL_MAX_DELAY);

	// read wavetable
	if(receive_buffer[0] != 0xaa) {
		return 0;
	}

	// set the address to read from now.
	set_address = byte_flip(WAVEFORM_DATA_START);
	HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)&set_address, 2, HAL_MAX_DELAY);
	// there is a wavetable, lets read
	HAL_I2C_Master_Receive(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)waveform, WAVEFORM_SIZE*2, HAL_MAX_DELAY);
	return 1;
}

void ui_setup(I2C_HandleTypeDef hi2c1,
		SPI_HandleTypeDef spi1,
		SPI_HandleTypeDef spi2
		//DMA_HandleTypeDef dma1
	) {
	storage_i2c = hi2c1;
	m4_spi = spi1;
	screen_spi = spi2;
	//m4_dma = dma1;

	const char msg_calibrate[] = "Touch each corner as the red square appears";

	// Dummy data to start
	reset_waveform();

	setSpi(spiSend, spiReceive, selectChip);
	setReset(resetChip);

	// this is probably unnecessary but it makes me feel better
	// additionally it gives me a debug point rn
	HAL_Delay(2000);

	resetChip();
	LCD_Initial();
	enableTouch();
	displayOn();
	fillScreen(0xffff); // make the screen all white
	if(!restoreCalibration()) {
		textMode();
		setTextColor(0x0000);
		setTextPosition(1023/2 - 8*(sizeof(msg_calibrate)), 511/2);
		screenWrite(msg_calibrate);
		graphicsMode();
		calibrate();
	}
}

uint8_t sendCalibration() {
	uint16_t send_buffer[1 + 2] = {CALIBRATION_DATA_START, 0, 0};
	int res;

	send_buffer[1] = scalex;
	send_buffer[2] = scaley;
	res = HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)send_buffer, 6, HAL_MAX_DELAY);
	if(res == HAL_ERROR) {
		return 0;
	}
	HAL_Delay(10);

	send_buffer[0] = CALIBRATION_DATA_SECOND;
	send_buffer[1] = divx;
	send_buffer[2] = divy;
	res = HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)send_buffer, 6, HAL_MAX_DELAY);
	if(res == HAL_ERROR) {
		return 0;
	}
	HAL_Delay(10);

	send_buffer[0] = CALIBRATION_DATA_ID;
	send_buffer[1] = 0xaaaa;
	res = HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)send_buffer, 3, HAL_MAX_DELAY);
	if(res == HAL_ERROR) {
		return 0;
	}
	HAL_Delay(10);
	return 1;
}

void screen_test() {
	// Delay 10 seconds because the test does not need to be constantly running.
		//HAL_Delay(1 * 1000);
		//HAL_SPI_Transmit_DMA(&m4_spi, (uint8_t *)waveform, WAVEFORM_SIZE);
		fillScreen(0xf800);
		drawRect(200,120,599,360, 0x07e0);
		HAL_Delay(1000);
		fillScreen(0x07e0);
		drawRect(200,120,599,360, 0x001f);
		HAL_Delay(1000);
		fillScreen(0x001f);
		drawRect(200,120,599,360, 0xf800);
		HAL_Delay(1000);
		drawLine(0, 0, 799, 479, 0x0000);
		drawLine(799, 0, 0, 479, 0x0000);
		HAL_Delay(1000);
		for(int i = 0; i < 800; i++) {
			uint16_t y = undoScale(waveform[find_location(i)]);
			drawPixel(i,y,0x0000);
		}
		HAL_Delay(5000);
}

void process_button() {
	button_press = 1;
}

void process_spi() {
	finish_spi_send = 1;
}

void draw_waveform_screen() {
	fillScreen(0xffff);
	for(int i = 0; i < 800; i++) {
		uint16_t y = undoScale(waveform[find_location(i)]);
		drawRect(i,y,i+2, y+2,0x0000);
		//drawPixel(i,y,0x0000);
	}
}
void wait_user_back_button() {
	drawRect(0,0,60, 40, 0x618c);
	textMode();
	setTextColor(0x0000);
	setTextPosition(20, 20);
	screenWrite("Back");
	graphicsMode();
	HAL_Delay(500);
	uint16_t tmpx, tmpy;
	while(!isTouchEvent());
	while(1) {
		if(isTouchEvent()) {
			readTouch(&tmpx, &tmpy);
			tmpx = (tmpx - scalex) * 800 / divx;
			tmpy = (tmpy - scaley) * 480 / divy;
			if((tmpx < 60) && (tmpy < 40)) break;
		}
	}
	while(isTouchEvent()) readTouch(&tmpx, &tmpy);
}
void usr_draw_waveform_loop() {
	//reset_waveform();
	//HAL_Delay(100);
	//const char msg_fail_save[] = "Failed to save waveform";
	fillScreen(0xffff);
	uint8_t change_made = 0;
	draw_waveform_screen();
	//calibrate();
	//HAL_Delay(5000);
	//while(1);
	while(!button_press) {
		int touch = isTouchEvent();
		if(touch) {
			if(change_made == 0) {
				reset_waveform();
				change_made = 1;
			}
			uint16_t tmpy, tmpx;
			readTouch(&tmpx, &tmpy);
			tmpx = (tmpx-scalex) * 800 / divx;
			tmpy = (tmpy-scaley) * 480 / divy;
			if(tmpx > 800) tmpx = 800;
			if(tmpy > 480) tmpy = 480;
			drawRect(tmpx, 0, tmpx+2, 479, 0xffff);
			drawRect(tmpx, tmpy, tmpx+2, tmpy+2, 0x0000);
			stream_touch_sample(tmpx, tmpy);
		}
	}

	interpolate_user_streamed_wave();
	scale_user_streamed();
	interpolate_waveform();
	normalize_waveform();
	button_press = 0;

	/*if(!sendWavetableI2C()) {
		textMode();
		setTextColor(0x0000);
		setTextPosition(1024/2, 512/2);
		screenWrite(msg_fail_save);
		graphicsMode();
		return;
	}*/
	draw_waveform_screen();

	wait_user_back_button();
	HAL_Delay(250);
	uint16_t tmp;
	if(isTouchEvent())
		while(isTouchEvent()) readTouch(&tmp, &tmp);
	unsaved_changes = 1;
}

enum menu_options main_menu_start(uint8_t start) {
	// options:
	// save, load, calibrate, clear, send
	const enum menu_options menu_items[] = {SAVE, LOAD, ASDR, CLEAR, CALIBRATE, SEND};
	const uint16_t color_options[] = {
			(17 << 11) | (31 << 5) | (0),
			(27 << 11) | (0 << 5) | (14),
			(0 << 11) | (20 << 5) | (28),
			(30 << 11) | (22 << 5) | (0),
			(0 << 11) | (28 << 5) | (12),
			(30 << 11) | (21 << 5) | (20)
	};
	const uint16_t x_locs[] = {
			380, 540, 700, 60, 220
	};
	const uint16_t rs[] = {
			90, 34, 30, 30, 34
	};
	uint8_t offset = start;

	fillScreen(0xffff);
	uint16_t tmp;
	while(isTouchEvent()) readTouch(&tmp, &tmp);
	HAL_Delay(500);
	enableTouch();
	button_press = 0;
	while(!button_press) {
		for(uint8_t i = 0; i < sizeof(menu_items); i++) {
			uint8_t ptr = (i+offset) % sizeof(menu_items);
			drawCircle(x_locs[i], 240, rs[i], color_options[ptr]);
		}

		textMode();
		setTextColor(0x0000);
		setTextPosition(360, 225);

		switch(menu_items[offset]) {
			case SAVE:
				screenWrite("Save");
				break;
			case LOAD:
				screenWrite("Load");
				break;
			case CALIBRATE:
				screenWrite("Cali.");
				break;
			case CLEAR:
				screenWrite("New");
				break;
			case SEND:
				screenWrite("Play");
				break;
			case ASDR:
				screenWrite("ASDR");
				break;
			case BACK:
				break;
		}
		graphicsMode();
		while(!button_press) {
			if(isTouchEvent()) {
				uint16_t tmpx, tmpy;
				readTouch(&tmpx, &tmpy);
				tmpx = (tmpx-scalex) * 800 / divx;
				tmpy = (tmpy-scaley) * 480/divy;
				if(tmpx > 460)
					offset = (offset + 1) % sizeof(menu_items);
				else if(tmpx < 270)
					offset = (offset + (sizeof(menu_items) - 1)) % sizeof(menu_items);
				else if(tmpy > 150 && tmpy < 330)// they select the option
					button_press = 1;
				// wait for user to stop touching
				HAL_Delay(250);
				while(isTouchEvent())
					readTouch(&tmpx, &tmpy);
				break;
			}
		}
	}
	button_press = 0;

	return menu_items[offset];
}

void freeze() {
	while(1);
}

void consumeTouch() {
	uint16_t tmp;
	readTouch(&tmp, &tmp);
}

void loading_screen_transmit() {
	textMode();
	setTextColor(0x0000);
	setTextPosition(300, 225);
	screenWrite("Saving, this may take a moment.");
	graphicsMode();
	drawRect(0,115,799,120, 0x0000);
	drawRect(0,180,799,185, 0x0000);
}
void drawStartRects() {

	drawRect(margin, STARTY, margin+width, STARTY+2+0xff, 0x0000);
	drawRect(margin + border, STARTY+border, margin+width-border, STARTY+0xff, 0xffff);

	drawRect(2*margin + width, STARTY, 2*(margin+width), STARTY+2+0xff, 0x0000);
	drawRect(2*margin + width+border, STARTY+border, 2*(margin+width)-border, STARTY+0xff, 0xffff);

	drawRect(3*margin + 2*width, STARTY, 3*(margin+width), STARTY+2+0xff, 0x0000);
	drawRect(3*margin + 2*width+border, STARTY+border, 3*(margin+width)-border, STARTY+0xff, 0xffff);

	drawRect(4*margin + 3*width, STARTY, 4*(margin+width), STARTY+2+0xff, 0x0000);
	drawRect(4*margin + 3*width+border, STARTY+border, 4*(margin+width)-border, STARTY+0xff, 0xffff);
}

void clearAsdrRects(uint8_t f) {
	if(f == 1) {
		drawRect(52, STARTY+2, 98, STARTY+0xff, 0xffff);
	} else if(f == 2) {

	} else if(f == 3) {

	} else {

	}
}

void asdr_screen() {
	fillScreen(0xffff);

	uint16_t tmpx, tmpy;

	uint16_t attack = STARTY + 0xff;
	uint16_t decay = STARTY + 0xff;
	uint16_t sustain = STARTY + 0xff;
	uint16_t release = STARTY + 0xff;
	drawStartRects();

	drawRect(0, 0, 60, 40, 0x618c);

	textMode();
	setTextColor(0x0000);
	setTextPosition(20, 20);
	screenWrite("Done");

	setTextPosition(120, 80);
	screenWrite("A");

	setTextPosition(270, 80);
	screenWrite("S");

	setTextPosition(420, 80);
	screenWrite("D");

	setTextPosition(570, 80);
	screenWrite("R");
	graphicsMode();

	while(1) {
		if(!isTouchEvent()) continue;
		readTouch(&tmpx, &tmpy);

		tmpx = (tmpx-scalex) * 800 / divx;
		tmpy = (tmpy-scaley) * 480 / divy;
		if(tmpx > 800) tmpx = 800;
		if(tmpy > 480) tmpy = 480;

		if(tmpy < STARTY) tmpy = STARTY;
		if(tmpy > (STARTY + 0xff)) tmpy = STARTY + 0xff;

		if(tmpx > margin && tmpx < margin+width) {
			if(attack > tmpy) {
				drawRect(margin+border, tmpy, margin+width-border, attack, 0xf800);
			} else {
				drawRect(margin+border, attack, margin+width-border, tmpy, 0xffff);
			}

			attack = tmpy;
		} else if(tmpx > (2*margin + width) && tmpx < 2*(margin+width)) {
			if(decay > tmpy) {
				drawRect(2*margin + 1*width+border, tmpy, 2*(margin+width)-border, decay, 0x07e0);
			} else {
				drawRect(2*margin + 1*width+border, decay, 2*(margin+width)-border, tmpy, 0xffff);
			}

			decay = tmpy;
		} else if(tmpx > (3*margin + 2*width) && tmpx < 3*(margin+width)) {
			if(sustain > tmpy) {
				drawRect(3*margin + 2*width+border, tmpy, 3*(margin+width)-border, sustain, 0x001f);
			} else {
				drawRect(3*margin + 2*width+border, sustain, 3*(margin+width)-border, tmpy, 0xffff);
			}

			sustain = tmpy;
		} else if(tmpx > (4*margin + 3*width) && tmpx < 4*(margin+width)) {
			if(release > tmpy) {
				drawRect(4*margin + 3*width+border, tmpy, 4*(margin+width)-border, release, 0xe6ab);
			} else {
				drawRect(4*margin + 3*width+border, release, 4*(margin+width)-border, tmpy, 0xffff);
			}

			release = tmpy;
		} else if(tmpx < 60 && tmpy < 110) {
			data_send_struct.attack = 0xff + STARTY - attack;
			data_send_struct.decay = 0xff + STARTY - decay;
			data_send_struct.sustain = 0xff + STARTY - sustain;
			data_send_struct.release = 0xff + STARTY - release;
			return;
		}
	}
}

void ui_loop() {
	const uint8_t save = 0;
	const uint8_t load = 1;
	const uint8_t asdr = 2;
	const uint8_t clear = 3;
	const uint8_t calilbrate = 4;
	const uint8_t send = 5;
	//transmit_wavetable();
	//usr_draw_waveform_loop();
	uint8_t start_option = 1;
	while(1) {
		enum menu_options chosen = main_menu_start(start_option);
		fillScreen(0xffff);
		start_option = 1;
		switch(chosen) {
			case SAVE:
				loading_screen_transmit();
				if(!sendWavetableI2C()) {
					fillScreen(0xf800);
					freeze();
				}
				start_option = send; // send
				break;
			case LOAD:
				if(!restoreWaveTableI2C()) {
					fillScreen(0xf800);
					freeze();
				}
				draw_waveform_screen();
				wait_user_touch();
				start_option = send; // send
				break;
			case SEND:
				transmit_wavetable();
				draw_waveform_screen();
				wait_user_back_button();
				start_option = save; //save
				break;
			case CALIBRATE:
				calibrate();
				if(!sendCalibration()) {
					fillScreen(0xf800);
					freeze();
				}
				start_option = clear; // new
				break;
			case CLEAR:
				reset_waveform();
				usr_draw_waveform_loop();
				start_option = send; // send
				break;
			case ASDR:
				asdr_screen();
				start_option = send;
				break;
			case BACK:
				break;
		}
		disableTouch();
		if(isTouchEvent()) consumeTouch();
	}

	fillScreen(0x001f);
	while(1);
}
