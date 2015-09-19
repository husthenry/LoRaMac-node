/*
 / _____)			 _				| |
( (____	_____ ____ _| |_ _____	____| |__
 \____ \| ___ |	(_	_) ___ |/ ___)	_ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
	(C)2013 Semtech

Description: SX1276 driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "radio.h"
#include "sx1276/sx1276.h"
#include "sx1276-board.h"

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
static bool RadioIsActive = false;

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
	.Init				= SX1276Init,
	.Status				= SX1276GetStatus,
	.SetModem			= SX1276SetModem,
	.SetChannel			= SX1276SetChannel,
	.IsChannelFree		= SX1276IsChannelFree,
	.Random				= SX1276Random,
	.SetRxConfig		= SX1276SetRxConfig,
	.SetTxConfig		= SX1276SetTxConfig,
	.CheckRfFrequency	= SX1276CheckRfFrequency,
	.TimeOnAir			= SX1276GetTimeOnAir,
	.Send				= SX1276Send,
	.Sleep				= SX1276SetSleep,
	.Standby			= SX1276SetStby, 
	.Rx					= SX1276SetRx,
	.StartCad			= SX1276StartCad,
	.Rssi				= SX1276ReadRssi,
	.Write				= SX1276Write,
	.Read				= SX1276Read,
	.WriteBuffer		= SX1276WriteBuffer,
	.ReadBuffer			= SX1276ReadBuffer
};

/*!
 * Antenna switch GPIO pins objects
 */
Gpio_t AntSwitchLf;
Gpio_t AntSwitchHf;

void SX1276IoInit( void )
{
	GpioInit( &SX1276.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );

	GpioInit( &SX1276.DIO0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
	GpioInit( &SX1276.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
	GpioInit( &SX1276.DIO2, RADIO_DIO_2, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
	GpioInit( &SX1276.DIO3, RADIO_DIO_3, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
	GpioInit( &SX1276.DIO4, RADIO_DIO_4, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
	GpioInit( &SX1276.DIO5, RADIO_DIO_5, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
}

void SX1276IoIrqInit( DioIrqHandler **irqHandlers )
{
	GpioSetInterrupt( &SX1276.DIO0, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, irqHandlers[0] );
	GpioSetInterrupt( &SX1276.DIO1, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, irqHandlers[1] );
	GpioSetInterrupt( &SX1276.DIO2, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, irqHandlers[2] );
	GpioSetInterrupt( &SX1276.DIO3, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, irqHandlers[3] );
	GpioSetInterrupt( &SX1276.DIO4, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, irqHandlers[4] );
	GpioSetInterrupt( &SX1276.DIO5, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, irqHandlers[5] );
}

void SX1276IoDeInit( void )
{
	GpioInit( &SX1276.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

	GpioInit( &SX1276.DIO0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &SX1276.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &SX1276.DIO2, RADIO_DIO_2, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &SX1276.DIO3, RADIO_DIO_3, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &SX1276.DIO4, RADIO_DIO_4, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &SX1276.DIO5, RADIO_DIO_5, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

uint8_t SX1276GetPaSelect( uint32_t channel )
{
	if ( channel < RF_MID_BAND_THRESH )
		return RF_PACONFIG_PASELECT_PABOOST;
	return RF_PACONFIG_PASELECT_RFO;
}

void SX1276SetAntSwLowPower( bool status )
{
	if ( RadioIsActive != status )
	{
		RadioIsActive = status;
	
		if ( status == false )
			SX1276AntSwInit( );
		else
			SX1276AntSwDeInit( );
	}
}

void SX1276AntSwInit( void )
{
#ifdef RADIO_ANT_SWITCH_LF
	GpioInit( &AntSwitchLf, RADIO_ANT_SWITCH_LF, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
#endif
#ifdef RADIO_ANT_SWITCH_HF
	GpioInit( &AntSwitchHf, RADIO_ANT_SWITCH_HF, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
#endif
}

void SX1276AntSwDeInit( void )
{
#ifdef RADIO_ANT_SWITCH_LF
	GpioInit( &AntSwitchLf, RADIO_ANT_SWITCH_LF, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
#endif
#ifdef RADIO_ANT_SWITCH_HF
	GpioInit( &AntSwitchHf, RADIO_ANT_SWITCH_HF, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
#endif
}

void SX1276SetAntSw( uint8_t rxTx )
{
	if ( SX1276.RxTx == rxTx )
		return;

	SX1276.RxTx = rxTx;

	if ( rxTx != 0 ) // 1: TX, 0: RX
	{
		GpioWrite( &AntSwitchLf, 0 );
		GpioWrite( &AntSwitchHf, 1 );
	}
	else
	{
		GpioWrite( &AntSwitchLf, 1 );
		GpioWrite( &AntSwitchHf, 0 );
	}
}

bool SX1276CheckRfFrequency( uint32_t frequency )
{
	// Implement check. Currently all frequencies are supported
	return true;
}
