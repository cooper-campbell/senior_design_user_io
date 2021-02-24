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

// Maps 0,799 to the location in waveform of the sample.
int find_location(int x) {
	int tmp = 2 * (x+1);
	return tmp + tmp/10 - tmp/100 -1;
}

int scale(uint16_t y) {
	int tmp = y - 240;
	return 136 * y + tmp/2;
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
		waveform[i] = i;
}

void ui_loop() {
	// Delay 10 seconds because the test does not need to be constantly running.
	HAL_Delay(1 * 1000);
	HAL_SPI_Transmit_DMA(&m4_spi, (uint8_t *)waveform, WAVEFORM_SIZE);
	while(1);
}

