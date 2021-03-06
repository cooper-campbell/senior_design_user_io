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

// Private vars
int16_t waveform[WAVEFORM_SIZE] = {0};
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
	HAL_SPI_Transmit(&m4_spi, (uint8_t *)waveform, WAVEFORM_SIZE*2, HAL_MAX_DELAY);
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
	drawRect(0, 475, 4, 479, 0xffff);;
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
		HAL_Delay(10);
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
	const char msg_restore[] = "Calibration settings restored, press anywhere to continue";
	const char msg_waveform_restore[] = "Restored waveform from previous session, press to continue";

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
	textMode();
	setTextColor(0x0000);
	if(restoreCalibration()) {
		setTextPosition(1023/2 - 8*(sizeof(msg_restore)), 511/2);
		screenWrite(msg_restore);
	} else {
		setTextPosition(1023/2 - 8*(sizeof(msg_calibrate)), 511/2);
		screenWrite(msg_calibrate);
		calibrate();
	}
	wait_user_input();

	// check for waveform.
	if(restoreWaveTableI2C()) {
		setTextPosition(1023/2 - 8*(sizeof(msg_waveform_restore)), 511/2-32);
		screenWrite(msg_waveform_restore);
		wait_user_input();
	}
	graphicsMode();
}


void sendCalibration() {
	uint16_t send_buffer[1 + 2] = {CALIBRATION_DATA_START, 0, 0};

	send_buffer[1] = scalex;
	send_buffer[2] = scaley;
	HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)send_buffer, 6, HAL_MAX_DELAY);

	send_buffer[0] = CALIBRATION_DATA_SECOND;
	send_buffer[1] = divx;
	send_buffer[2] = divy;
	HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)send_buffer, 6, HAL_MAX_DELAY);

	send_buffer[0] = CALIBRATION_DATA_ID;
	send_buffer[1] = 0xaaaa;
	HAL_I2C_Master_Transmit(&storage_i2c, EEPROM_ADDRESS_WRITE, (uint8_t *)send_buffer, 3, HAL_MAX_DELAY);
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

void usr_draw_waveform_loop() {
	//reset_waveform();
	//HAL_Delay(100);
	const char msg_fail_save[] = "Failed to save waveform";
	fillScreen(0xffff);
	uint8_t change_made = 0;

	//calibrate();
	//HAL_Delay(5000);
	for(int i = 0; i < 800; i++) {
		uint16_t y = undoScale(waveform[find_location(i)]);
		drawRect(i,y,i+2, y+2,0x0000);
		//drawPixel(i,y,0x0000);
	}
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
	fillScreen(0xffff);
	if(!sendWavetableI2C()) {
		textMode();
		setTextColor(0x0000);
		setTextPosition(1024/2, 512/2);
		screenWrite(msg_fail_save);
		graphicsMode();
		return;
	}
	for(int i = 0; i < 800; i++) {
		uint16_t y = undoScale(waveform[find_location(i)]);
		drawRect(i,y,i+2, y+2,0x0000);
		//drawPixel(i,y,0x0000);
	}
}

void ui_loop() {
	//transmit_wavetable();
	usr_draw_waveform_loop();
	while(1);
}
