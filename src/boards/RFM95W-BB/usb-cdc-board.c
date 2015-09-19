
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "board.h"
#include "usb_pwr.h"

#include "stm32l1xx_exti.h"

extern LINE_CODING linecoding;

static void IntToUnicode( uint32_t value , uint8_t *pbuf , uint8_t len )
{
	uint8_t ch;
	while( len != 0 )
	{
		--len;
		ch = (uint8_t)(value >> 28);
		if( ch < 0xA )
			ch += '0';
		else
			ch += ('A' - 10);
		*pbuf++ = ch;
		value = value << 4;
		*pbuf++ = 0;
	}
}

void UsbMcuInit( void )
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	/* Enable the SYSCFG module clock */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG, ENABLE );

	/* Configure the EXTI line 18 connected internally to the USB IP */
	EXTI_ClearITPendingBit( EXTI_Line18 );
	EXTI_InitStructure.EXTI_Line = EXTI_Line18;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init( &EXTI_InitStructure );

	/* Enable USB clock */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USB, ENABLE );

	/* 2 bit for pre-emption priority, 2 bits for subpriority */
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_2 );

	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	/* Enable the USB Wake-up interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USB_FS_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	USB_Init( );
}

void UsbMcuEnterLowPowerMode( void )
{
	/* Set the device state to suspend */
	bDeviceState = SUSPENDED;
}

void UsbMcuLeaveLowPowerMode( void )
{
	DEVICE_INFO *pInfo = &Device_Info;

	/* Set the device state to the correct state */
	if( pInfo->Current_Configuration != 0 )
		/* Device configured */
		bDeviceState = CONFIGURED;
	else
		bDeviceState = ATTACHED;
}

void UsbMcuCableConfig( FunctionalState newState )
{
	if( newState != DISABLE )
		SYSCFG_USBPuCmd( ENABLE );
	else
		SYSCFG_USBPuCmd( DISABLE );
}

void UsbMcuGetSerialNum( void )
{
	uint32_t deviceSerial0, deviceSerial1, deviceSerial2;

	deviceSerial0 = *( uint32_t* )ID1;
	deviceSerial1 = *( uint32_t* )ID2;
	deviceSerial2 = *( uint32_t* )ID3;

	deviceSerial0 += deviceSerial2;

	if( deviceSerial0 != 0 )
	{
		IntToUnicode( deviceSerial0, &Virtual_Com_Port_StringSerial[2] , 8 );
		IntToUnicode( deviceSerial1, &Virtual_Com_Port_StringSerial[18], 4 );
	}
}

bool UsbMcuIsDeviceConfigured( void )
{
	return Virtual_ComPort_IsOpen( );
}

void USB_LP_IRQHandler(void)
{
	USB_Istr( );
}

void USB_FS_WKUP_IRQHandler(void)
{
	EXTI_ClearITPendingBit( EXTI_Line18 );
}
