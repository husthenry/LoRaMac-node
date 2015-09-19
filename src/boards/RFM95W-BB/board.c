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

#include "board.h"
#include "adc-board.h"

/*!
 * IO Extander pins objects
 */
Gpio_t IrqMpl3115;
Gpio_t IrqMag3110;
Gpio_t GpsPowerEn;
Gpio_t RadioPushButton;
Gpio_t BoardPowerDown;
Gpio_t NcIoe5;
Gpio_t NcIoe6;
Gpio_t NcIoe7;
Gpio_t NIrqSx9500;
Gpio_t Irq1Mma8451;
Gpio_t Irq2Mma8451;
Gpio_t TxEnSx9500;
Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;
Gpio_t Led4;

/*
 * MCU objects
 */
Gpio_t GpsPps;
Gpio_t GpsRx;
Gpio_t GpsTx;
Gpio_t UsbDetect;
Gpio_t Wkup1;
Gpio_t DcDcEnable;
Gpio_t BatVal;

Adc_t Adc;
I2c_t I2c;
Uart_t Uart1;
#if defined( USE_USB_CDC )
	Uart_t UartUsb;
#endif

/*!
 * Initializes the unused GPIO to a know status
 */
static void BoardUnusedIoInit( void );

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

void BoardInitPeriph( void )
{
	/* Init the GPIO extender pins */
	GpioInit( &GpsPowerEn, GPS_POWER_ON, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
	GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &Led3, LED_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

	// Init GPS
	GpsInit( );

	// Switch LED 1, 2, 3, 4 OFF
	GpioWrite( &Led1, 1 );
	GpioWrite( &Led2, 1 );
	GpioWrite( &Led3, 1 );
	GpioWrite( &Led4, 1 );
}

void BoardInitMcu( void )
{
	if ( ! McuInitialized )
	{
#if defined( USE_BOOTLOADER )
		#warning "Set the Vector Table base location at 0x4000"
		// Set the Vector Table base location at 0x4000
		NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x4000 );
#endif
		// We use IRQ priority group 4 for the entire project
		// When setting the IRQ, only the preemption priority is used
		NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

		// Disable Systick
		SysTick->CTRL	&= ~SysTick_CTRL_TICKINT_Msk;	// Systick IRQ off 
		SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;			// Clear SysTick Exception pending flag

		SpiInit( &SX1276.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
		SX1276IoInit( );

#if defined( USE_USB_CDC )
		{
			Gpio_t usbDM;
			GpioInit( &usbDM, USB_DM, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );

			if ( GpioRead( &usbDM ) == 0 )
			{
				TimerSetLowPowerEnable( false );
				UsbMcuInit( );
				UartInit( &UartUsb, UART_USB_CDC, NC, NC );
				UartConfig( &UartUsb, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
			}
			else
			{
				TimerSetLowPowerEnable( true );
				GpioInit( &usbDM, USB_DM, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
			}
		}

#elif ( LOW_POWER_MODE_ENABLE )
		TimerSetLowPowerEnable( true );
#else
		TimerSetLowPowerEnable( false );
#endif
		BoardUnusedIoInit( );

		if ( TimerGetLowPowerEnable( ) == true )
			RtcInit( );
		else
			TimerHwInit( );

		McuInitialized = true;
	}
}

void BoardDeInitMcu( void )
{
	Gpio_t ioPin;

	SpiDeInit( &SX1276.Spi );
	SX1276IoDeInit( );

	GpioInit( &ioPin, OSC_HSE_IN, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &ioPin, OSC_HSE_OUT, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

	GpioInit( &ioPin, OSC_LSE_IN, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &ioPin, OSC_LSE_OUT, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	
	McuInitialized = false;
}

void BoardGetUniqueId( uint8_t *id )
{
	id[0] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
	id[1] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
	id[2] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
	id[3] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
	id[4] = ( ( *( uint32_t* )ID2 ) ) >> 24;
	id[5] = ( ( *( uint32_t* )ID2 ) ) >> 16;
	id[6] = ( ( *( uint32_t* )ID2 ) ) >> 8;
	id[7] = ( ( *( uint32_t* )ID2 ) );
}

/*!
 * Factory power supply
 */
#define FACTORY_POWER_SUPPLY	3.0L

/*!
 * VREF calibration value
 */
#define VREFINT_CAL				( *( uint16_t* )0x1FF80078 )

/*!
 * ADC maximum value
 */
#define ADC_MAX_VALUE			4096

/*!												 
 * Battery thresholds								
 */												 
#define BATTERY_MAX_LEVEL		4150 // mV
#define BATTERY_MIN_LEVEL		3200 // mV
#define BATTERY_SHUTDOWN_LEVEL	3100 // mV

uint8_t BoardMeasureBatterieLevel( void ) 
{
	return 254;
}

static void BoardUnusedIoInit( void )
{
	Gpio_t ioPin;
	
	/* USB */
#if !defined( USE_USB_CDC )
	GpioInit( &ioPin, USB_DM, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &ioPin, USB_DP, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

	GpioInit( &ioPin, BOOT_1, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );

#if defined( USE_DEBUGGER )
	DBGMCU_Config( DBGMCU_SLEEP, ENABLE );
	DBGMCU_Config( DBGMCU_STOP, ENABLE);
	DBGMCU_Config( DBGMCU_STANDBY, ENABLE);
#else
	DBGMCU_Config( DBGMCU_SLEEP, DISABLE );
	DBGMCU_Config( DBGMCU_STOP, DISABLE );
	DBGMCU_Config( DBGMCU_STANDBY, DISABLE );

	GpioInit( &ioPin, SWDIO, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &ioPin, SWCLK, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}
