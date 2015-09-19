/*
 / _____)			 _				| |
( (____	_____ ____ _| |_ _____	____| |__
 \____ \| ___ |	(_	_) ___ |/ ___)	_ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
	(C)2013 Semtech

Description: Generic low level driver for GPS receiver

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

/*!
 * FIFO buffers size
 */
//#define FIFO_TX_SIZE	128
#define FIFO_RX_SIZE	128

//uint8_t TxBuffer[FIFO_TX_SIZE];
uint8_t RxBuffer[FIFO_RX_SIZE];

int8_t NmeaString[128];
uint8_t NmeaStringSize = 0;

void GpsMcuInit( void )
{
	NmeaStringSize = 0;

	// FifoInit( &Uart1.FifoTx, TxBuffer, FIFO_TX_SIZE );
	FifoInit( &Uart1.FifoRx, RxBuffer, FIFO_RX_SIZE );
	Uart1.IrqNotify = GpsMcuIrqNotify;

	// GpioWrite( &GpsPowerEn, 0 );	// power down the GPS
	GpioWrite( &GpsPowerEn, 1 );	// power up the GPS
}

void GpsMcuIrqNotify( UartNotifyId_t id )
{
	uint8_t data;
	if ( id == UART_NOTIFY_RX )
		if ( UartGetChar( &Uart1, &data ) == 0 )
		{
			if ( data == '$' || NmeaStringSize >= 128 )
				NmeaStringSize = 0;

			NmeaString[NmeaStringSize++] = data;

			if ( data == '\n' )
			{
				NmeaString[NmeaStringSize] = '\0';
				GpsParseGpsData( NmeaString, NmeaStringSize );
				UartDeInit( &Uart1 );
				BlockLowPowerDuringTask ( false );
			}
		}
}
