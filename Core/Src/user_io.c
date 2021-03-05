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

// Private vars
int16_t waveform[WAVEFORM_SIZE] = {0};
// anonymous structure to count waveform properly
// almost out of ram already so time for bitfields
I2C_HandleTypeDef storage_i2c;
SPI_HandleTypeDef m4_spi;
SPI_HandleTypeDef screen_spi;
DMA_HandleTypeDef m4_dma;

uint16_t scalex = 0;
uint16_t scaley = 0;
uint16_t divx = 1;
uint16_t divy = 1;

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

void stream_touch_sample(uint16_t x, uint16_t y) {
	int location = find_location(x);
	waveform[location] = scale(y);
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

void ui_setup(I2C_HandleTypeDef hi2c1,
		SPI_HandleTypeDef spi1,
		SPI_HandleTypeDef spi2,
		DMA_HandleTypeDef dma1
	) {
	storage_i2c = hi2c1;
	m4_spi = spi1;
	screen_spi = spi2;
	m4_dma = dma1;
	// Dummy data to start
	for(int i = 0; i < WAVEFORM_SIZE; i++)
		waveform[i] = sin_wave[i];

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
}

void sendWavetableI2C() {
	// eeprom supports 32 bit writes
	// need 16 bits for the 12 address bits
	uint16_t send_buffer[1 + 2] = {WAVEFORM_DATA_START, 0, 0};

	// send wavetable
	for(int i = 0; i < WAVEFORM_SIZE; i+=2) {
		// update the write address	
		send_buffer[0] = WAVEFORM_DATA_START + i * 4;

		// move over waveform information to the buffer
		send_buffer[1] = waveform[i];
		send_buffer[2] = waveform[i+1];
		
		HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)send_buffer, 6, HAL_MAX_DELAY);
	}
	// set confirmation of wavetable
	send_buffer[0] = 0x0101;
	HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)send_buffer, 1, HAL_MAX_DELAY);
}

uint8_t restoreWaveTableI2C() {
	// check if there is a wavetable
	uint8_t receive_buffer[1] = {0};
	uint16_t set_address = WAVEFORM_ID_ADDR;
	HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)&set_address, 2, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)receive_buffer, 1, HAL_MAX_DELAY);

	// read wavetable
	if(receive_buffer[0] != 0x01) {
		return 0;
	}

	// set the address to read from now.
	set_address = WAVEFORM_DATA_START;
	HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)&set_address, 2, HAL_MAX_DELAY);
	// there is a wavetable, lets read
	HAL_I2C_Master_Receive(&storage_i2c, EEPROM_ADDRESS_READ, (uint8_t *)waveform, WAVEFORM_SIZE*2, HAL_MAX_DELAY);
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

void calibrate() {
	uint16_t rectx1, recty1, rectx2, recty2, rectx3, recty3, rectx4, recty4;

	fillScreen(0xffff);
	drawRect(0, 0, 4, 4, 0xf800);
	while(!isTouchEvent());
	HAL_Delay(500);
	readTouch(&rectx1, &recty1);
	fillScreen(0xffff);
	HAL_Delay(1000);

	drawRect(795, 0, 799, 4, 0xf800);
	while(!isTouchEvent());
	HAL_Delay(500);
	readTouch(&rectx2, &recty2);
	fillScreen(0xffff);
	HAL_Delay(1000);

	drawRect(795, 475, 799, 479, 0xf800);
	while(!isTouchEvent());
	HAL_Delay(500);
	readTouch(&rectx3, &recty3);
	fillScreen(0xffff);
	HAL_Delay(1000);

	drawRect(0, 475, 4, 479, 0xf800);
	while(!isTouchEvent());
	HAL_Delay(500);
	readTouch(&rectx4, &recty4);
	fillScreen(0xffff);
	HAL_Delay(1000);

}
void ui_loop() {
	//HAL_Delay(100);
	fillScreen(0xffff);
	const uint16_t width = 800;
	const uint16_t height = 480;
	uint16_t max_x = 0;
	uint16_t min_x = 799;
	uint16_t max_y = 0;
	uint16_t min_y = 479;

	//calibrate();
	//HAL_Delay(5000);
	for(int i = 0; i < 800; i++) {
		uint16_t y = undoScale(waveform[find_location(i)]);
		drawRect(i,y,i+2, y+2,0x0000);
	}
	//while(1);
	int count = 0;
	while(1000) {
		int touch = isTouchEvent();
		if(touch) {
			uint16_t tmpy, tmpx;
			readTouch(&tmpx, &tmpy);
			tmpx = (tmpx-30) * 800 / 950;
			tmpy = (tmpy-130) * 480 / 818;
			if(tmpx > max_x) max_x = tmpx;
			if(tmpx < min_x) min_x = tmpx;
			if(tmpy > max_y) max_y = tmpy;
			if(tmpy < min_y) min_y = tmpy;
			drawRect(tmpx, 0, tmpx+2, 479, 0xffff);
			drawRect(tmpx, tmpy, tmpx+2, tmpy+2, 0x0000);
			stream_touch_sample(tmpx, tmpy);
			count++;
		}
	}
	interpolate_waveform();
	normalize_waveform();

	//HAL_Delay(3000);
}

