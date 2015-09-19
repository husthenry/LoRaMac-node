/*
 / _____)			 _				| |
( (____	_____ ____ _| |_ _____	____| |__
 \____ \| ___ |	(_	_) ___ |/ ___)	_ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
	(C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "stm32l1xx.h"
#include "utilities.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "radio.h"
#include "sx1276/sx1276.h"
#include "gps.h"
#include "gps-board.h"
#include "rtc-board.h"
#include "timer-board.h"
#include "sx1276-board.h"
#include "uart-board.h"

#if defined( USE_USB_CDC )
	#include "usb-cdc-board.h"
#endif

/*!
 * NULL definition
 */
#ifndef NULL
	#define NULL	( ( void * )0 )
#endif

/*!
 * Generic definition
 */
#ifndef SUCCESS
	#define SUCCESS	1
#endif

#ifndef FAIL
	#define FAIL	0	
#endif

/*!
 * Unique Devices IDs register set ( STM32L1xxx )
 */
#define		 ID1	( 0x1FF80050 )
#define		 ID2	( 0x1FF80054 )
#define		 ID3	( 0x1FF80064 )

/*!
 * Random seed generated using the MCU Unique ID
 */
#define RAND_SEED	(	( *( uint32_t* )ID1 ) ^ \
						( *( uint32_t* )ID2 ) ^ \
						( *( uint32_t* )ID3 ) )

#define LED_1	PB_10
#define LED_2	PB_11
#define LED_3	PA_3

/*!
 * Board MCU pins definitions
 */

#define RADIO_RESET		PB_10

#define RADIO_MOSI		PA_7
#define RADIO_MISO		PA_6
#define RADIO_SCLK		PA_5
#define RADIO_NSS		PA_4

#define RADIO_DIO_0		PB_11
#define RADIO_DIO_1		PC_13
#define RADIO_DIO_2		PB_9
#define RADIO_DIO_3		PB_4
#define RADIO_DIO_4		PB_3
#define RADIO_DIO_5		PA_15

#define OSC_LSE_IN		PC_14
#define OSC_LSE_OUT		PC_15

#define OSC_HSE_IN		PH_0
#define OSC_HSE_OUT		PH_1

#define USB_DM			PA_11
#define USB_DP			PA_12

#define I2C_SCL			PB_6
#define I2C_SDA			PB_7

#define BOOT_1			PB_2
	
#define GPS_POWER_ON	PB_1
#define UART_TX			PA_9
#define UART_RX			PA_10	

#define SWDIO			PA_13
#define SWCLK			PA_14

/*!
 * LED GPIO pins objects
 */
extern Gpio_t GpsPowerEn;
extern Gpio_t Led1;
extern Gpio_t Led2;
extern Gpio_t Led3;

/*!
 * MCU objects
 */
extern Adc_t Adc;
extern Uart_t Uart1;
#if defined( USE_USB_CDC )
	extern Uart_t UartUsb;
#endif

extern Gpio_t GpsRx;
extern Gpio_t GpsTx;

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *		consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Measure the Battery level
 *
 * \retval value	battery level ( 0: very low, 254: fully charged )
 */
uint8_t BoardMeasureBatterieLevel( void );

/*!
 * \brief Gets the board 64 bits unique ID 
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

#endif // __BOARD_H__
