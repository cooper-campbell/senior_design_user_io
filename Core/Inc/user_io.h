/*
 * user_io.h
 *
 *  Created on: Feb 14, 2021
 *      Author: cooper
 */

#ifndef INC_USER_IO_H_
#define INC_USER_IO_H_

// Includes
#include "stm32f0xx_hal.h"

// Defines
#define WAVEFORM_SIZE 1746
// Prototypes

// called once in main to initialize the setup process.
void ui_setup(I2C_HandleTypeDef hi2c1,
		SPI_HandleTypeDef m4_spi,
		SPI_HandleTypeDef screen_spi,
		DMA_HandleTypeDef m4_dma
	);

// called from main in an infinite loop.
void ui_loop();

#endif /* INC_USER_IO_H_ */
