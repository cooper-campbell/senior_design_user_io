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
I2C_HandleTypeDef storage_i2c;
SPI_HandleTypeDef m4_spi;
SPI_HandleTypeDef screen_spi;
DMA_HandleTypeDef m4_dma;

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

