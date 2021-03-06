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
#include "default_wave.h"
#include "ra8875.h"

// Defines
#define WAVEFORM_SIZE 1746

#define LcdSyncReset_Pin GPIO_PIN_8
#define LcdSyncReset_GPIO_Port GPIOA
#define ScreenSelect_Pin GPIO_PIN_12
#define ScreenSelect_GPIO_Port GPIOB
#define SPI_NSS_Pin GPIO_PIN_4
#define SPI_NSS_GPIO_Port GPIOA

#define EEPROM_ADDRESS_WRITE 0xa0
#define EEPROM_ADDRESS_READ 0xa1

// Where I will store whether or not a waveform
// is actually in the eeprom
#define WAVEFORM_ID_ADDR 0x0000
// where the waveform data starts
#define WAVEFORM_DATA_START 0x0001
// where the calibration information will be saved.
// it has to be after the wavetable
// that is, to say, after 1 + 1746*2

// byte ordering: 0xDAC needs to be 0xAC0D
#define CALIBRATION_DATA_ID 0xAC0D
#define CALIBRATION_DATA_START 0xAD0D
#define CALIBRATION_DATA_SECOND 0xB10D

// Prototypes

// called once in main to initialize the setup process.
void ui_setup(I2C_HandleTypeDef hi2c1,
		SPI_HandleTypeDef m4_spi,
		SPI_HandleTypeDef screen_spi
		//DMA_HandleTypeDef m4_dma
	);

// called from main in an infinite loop.
void ui_loop();

void process_button();

void process_spi();

#endif /* INC_USER_IO_H_ */
