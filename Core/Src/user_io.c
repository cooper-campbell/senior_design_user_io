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
uint16_t waveform[WAVEFORM_SIZE] = {0};
// anonymous structure to count waveform properly
// almost out of ram already so time for bitfields
struct {
	uint32_t sample_counter : 4;
	uint32_t removal_counter : 4;
	uint32_t total_counter : 12;
} waveform_counter;
I2C_HandleTypeDef storage_i2c;
SPI_HandleTypeDef m4_spi;
SPI_HandleTypeDef screen_spi;
DMA_HandleTypeDef m4_dma;

// private declarations
void reset_counters() {
	waveform_counter.sample_counter = 0;
	waveform_counter.removal_counter = 0;
	waveform_counter.total_counter = 0;
}
void stream_touch_sample(uint16_t x, uint16_t y) {
	int location;
	// This is the math necessary to expand 800 samples into 1746.
	// Need to manually add 2 points at the end to make it match the beginning.
	int tmp = 2 * (x+1);
	location = tmp + tmp/10 - tmp/100 - 1;

	// The y math scales the 480 pixels to be [0, 2^16-1] as per the specified interface.
	waveform[location] = 136 * y + 17 * y/32;
}

void interpolate_waveform() {
	reset_counters();
	// So we double every sample, by adding 1 sample before it
	// Next, every 10 samples we add a new one, skipping every 100 samples
	uint16_t value_before = waveform[WAVEFORM_SIZE-1];
	uint16_t value_after;
	uint8_t inc = 1;

	for(unsigned int i = 0; i < WAVEFORM_SIZE-2; i += inc) {
		uint16_t value_after = waveform[i+1];
		inc = 1;
		// Increment sample.
		waveform_counter.sample_counter = (waveform_counter.sample_counter + 1)%10;

		// We may need to add two samples between these points.
		if(waveform_counter.sample_counter == 9) {
			// only if the removal counter is 0 do we skip this.
			if(waveform_counter.removal_counter != 0) {
				value_after = waveform[i+2];
				waveform[i] = (value_before + value_after)/3;
				waveform[i+1] = 2*(value_before + value_after)/3;
				// we need to skip the next sample since we just calculated it.
				inc = 2;
			}
			waveform_counter.removal_counter = (waveform_counter.removal_counter + 1) % 10;
		}
		// add a sample before every other sample received from screen.
		else if(i%2 == 0) {
			// simple linear interpolation here.
			waveform[i] = (value_before + value_after) / 2;
		}

		value_before = waveform[i];
		waveform_counter.total_counter++;
	}
	value_after = waveform[0];
	value_before = waveform[WAVEFORM_SIZE-3];
	// add final two points to smooth towards the beginning.
	waveform[WAVEFORM_SIZE-2] = (value_before + value_after) / 3;
	waveform[WAVEFORM_SIZE-1] = 2*(value_before + value_after) / 3;
	// Note that with this method, there are 3 interpolated points between the first input by the user and the last.
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
	HAL_Delay(10 * 1000);
	HAL_SPI_Transmit_DMA(&m4_spi, (uint8_t *)waveform, WAVEFORM_SIZE);
}

