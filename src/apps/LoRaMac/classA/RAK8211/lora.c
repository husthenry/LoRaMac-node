#include "board.h"
#include "Comissioning.h"
#include "LoRaMac.h"

/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            5000

 /*!
  * Defines a random delay for application data transmission duty cycle. 1s,
  * value in [ms].
  */
#define APP_TX_DUTYCYCLE_RND                        5000

  /*!
   * Default datarate
   */
#define LORAWAN_DEFAULT_DATARATE                    DR_0

   /*!
	* LoRaWAN confirmed messages
	*/
#define LORAWAN_CONFIRMED_MSG_ON                    false

	/*!
	 * LoRaWAN Adaptive Data Rate
	 *
	 * \remark Please note that when ADR is enabled the end-device should be static
	 */
#define LORAWAN_ADR_ON                              1

#if defined( USE_BAND_868 )

#include "LoRaMacTest.h"

	 /*!
	  * LoRaWAN ETSI duty cycle control enable/disable
	  *
	  * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
	  */
#define LORAWAN_DUTYCYCLE_ON                        false

#define USE_SEMTECH_DEFAULT_CHANNEL_LINEUP          1

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 )

#define LC4                { 867100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 867300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 867500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 867700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 867900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC9                { 868800000, { ( ( DR_7 << 4 ) | DR_7 ) }, 2 }
#define LC10               { 868300000, { ( ( DR_6 << 4 ) | DR_6 ) }, 1 }

#endif

#endif

/*!
* LoRaWAN application port
*/
#define LORAWAN_APP_PORT			2

	   /*!
		* User application data buffer size
		*/
#if defined( USE_BAND_433 ) || defined( USE_BAND_470 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )

	#define LORAWAN_APP_DATA_SIZE	16

#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )

	#define LORAWAN_APP_DATA_SIZE	11

#endif

static uint8_t DevEui[] = LORAWAN_DEVICE_EUI;
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

/*!
 * Specifies the state of the application LED
 */
static bool AppLedStateOn = false;

/*!
 * Application port
 */
static uint8_t AppPort = LORAWAN_APP_PORT;

/*!
 * User application data size
 */
static uint8_t AppDataSize = LORAWAN_APP_DATA_SIZE;

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_MAX_SIZE	64

/*!
 * User application data
 */
static uint8_t AppData[LORAWAN_APP_DATA_MAX_SIZE];

/*!
 * Indicates if the node is sending confirmed or unconfirmed messages
 */
static uint8_t IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;

/*!
 * Defines the application data transmission duty cycle
 */
static uint32_t TxDutyCycleTime;

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxNextPacketTimer;

/*!
 * Indicates if a new packet can be sent
 */
static bool NextTx = true;

/*!
 * Device states
 */
static enum eDeviceState
{
	DEVICE_STATE_INIT,
	DEVICE_STATE_JOIN,
	DEVICE_STATE_SEND,
	DEVICE_STATE_CYCLE,
	DEVICE_STATE_SLEEP
} DeviceState;


/*!
 * \brief   Prepares the payload of the frame
 */
static void PrepareTxFrame(uint8_t port)
{
    switch( port )
    {
    case 2:
        {
#if defined( USE_BAND_433 ) || defined( USE_BAND_470 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
			uint16_t pressure = 0;
			int16_t altitudeBar = 0;
			int16_t temperature = 0;
			int32_t latitude, longitude = 0;
			int16_t altitudeGps = 0xFFFF;
			uint8_t batteryLevel = 0;
			/*
			pressure = ( uint16_t )( MPL3115ReadPressure( ) / 10 );             // in hPa / 10
			temperature = ( int16_t )( MPL3115ReadTemperature( ) * 100 );       // in �C * 100
			altitudeBar = ( int16_t )( MPL3115ReadAltitude( ) * 10 );           // in m * 10
			batteryLevel = BoardGetBatteryLevel( );                             // 1 (very low) to 254 (fully charged)
			GpsGetLatestGpsPositionBinary( &latitude, &longitude );
			altitudeGps = GpsGetLatestGpsAltitude( );                           // in m
			*/
			AppData[0] = AppLedStateOn;
			AppData[1] = ( pressure >> 8 ) & 0xFF;
			AppData[2] = pressure & 0xFF;
			AppData[3] = ( temperature >> 8 ) & 0xFF;
			AppData[4] = temperature & 0xFF;
			AppData[5] = ( altitudeBar >> 8 ) & 0xFF;
			AppData[6] = altitudeBar & 0xFF;
			AppData[7] = batteryLevel;
			AppData[8] = ( latitude >> 16 ) & 0xFF;
			AppData[9] = ( latitude >> 8 ) & 0xFF;
			AppData[10] = latitude & 0xFF;
			AppData[11] = ( longitude >> 16 ) & 0xFF;
			AppData[12] = ( longitude >> 8 ) & 0xFF;
			AppData[13] = longitude & 0xFF;
			AppData[14] = ( altitudeGps >> 8 ) & 0xFF;
			AppData[15] = altitudeGps & 0xFF;

			AppDataSize = 16;

#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )

			int16_t temperature = 0;
			int32_t latitude, longitude = 0;
			uint16_t altitudeGps = 0xFFFF;
			uint8_t batteryLevel = 0;
			/*
			temperature = ( int16_t )( MPL3115ReadTemperature( ) * 100 );       // in �C * 100

			batteryLevel = BoardGetBatteryLevel( );                             // 1 (very low) to 254 (fully charged)
			GpsGetLatestGpsPositionBinary( &latitude, &longitude );
			altitudeGps = GpsGetLatestGpsAltitude( );                           // in m
			*/
			AppData[0] = AppLedStateOn;
			AppData[1] = temperature;                                           // Signed degrees celsius in half degree units. So,  +/-63 C
			AppData[2] = batteryLevel;                                          // Per LoRaWAN spec; 0=Charging; 1...254 = level, 255 = N/A
			AppData[3] = ( latitude >> 16 ) & 0xFF;
			AppData[4] = ( latitude >> 8 ) & 0xFF;
			AppData[5] = latitude & 0xFF;
			AppData[6] = ( longitude >> 16 ) & 0xFF;
			AppData[7] = ( longitude >> 8 ) & 0xFF;
			AppData[8] = longitude & 0xFF;
			AppData[9] = ( altitudeGps >> 8 ) & 0xFF;
			AppData[10] = altitudeGps & 0xFF;

			AppDataSize = 11;

#endif
        }
        break;
    case 224:
		AppData[0] = 0xAA;
		AppData[1] = 0x66;

		AppDataSize = 2;

		break;
    default:
        break;
    }
}

/*!
 * \brief   Prepares the payload of the frame
 *
 * \retval  [0: frame could be send, 1: error]
 */
static bool SendFrame(void)
{
	McpsReq_t mcpsReq;
	LoRaMacTxInfo_t txInfo;

	if (LoRaMacQueryTxPossible(AppDataSize, &txInfo) != LORAMAC_STATUS_OK)
	{
		// Send empty frame in order to flush MAC commands
		mcpsReq.Type = MCPS_UNCONFIRMED;
		mcpsReq.Req.Unconfirmed.fBuffer = NULL;
		mcpsReq.Req.Unconfirmed.fBufferSize = 0;
		mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
	}
	else
	{
		if (IsTxConfirmed == false)
		{
			mcpsReq.Type = MCPS_UNCONFIRMED;
			mcpsReq.Req.Unconfirmed.fPort = AppPort;
			mcpsReq.Req.Unconfirmed.fBuffer = AppData;
			mcpsReq.Req.Unconfirmed.fBufferSize = AppDataSize;
			mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
		}
		else
		{
			mcpsReq.Type = MCPS_CONFIRMED;
			mcpsReq.Req.Confirmed.fPort = AppPort;
			mcpsReq.Req.Confirmed.fBuffer = AppData;
			mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
			mcpsReq.Req.Confirmed.NbTrials = 8;
			mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
		}
	}

	if (LoRaMacMcpsRequest(&mcpsReq) == LORAMAC_STATUS_OK)
	{
		return false;
	}
	return true;
}

/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static void OnTxNextPacketTimerEvent(void)
{
	MibRequestConfirm_t mibReq;
	LoRaMacStatus_t status;

	TimerStop(&TxNextPacketTimer);

	mibReq.Type = MIB_NETWORK_JOINED;
	status = LoRaMacMibGetRequestConfirm(&mibReq);

	if (status == LORAMAC_STATUS_OK)
	{
		if (mibReq.Param.IsNetworkJoined == true)
		{
			DeviceState = DEVICE_STATE_SEND;
			NextTx = true;
		}
		else
		{
			DeviceState = DEVICE_STATE_JOIN;
		}
	}
}

/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] mcpsConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void McpsConfirm(McpsConfirm_t *mcpsConfirm)
{
	if (mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
	{
		switch (mcpsConfirm->McpsRequest)
		{
		case MCPS_UNCONFIRMED:
		{
			// Check Datarate
			// Check TxPower
			break;
		}
		case MCPS_CONFIRMED:
		{
			// Check Datarate
			// Check TxPower
			// Check AckReceived
			// Check NbTrials
			break;
		}
		case MCPS_PROPRIETARY:
		{
			break;
		}
		default:
			break;
		}
	}
	NextTx = true;
}

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void McpsIndication(McpsIndication_t *mcpsIndication)
{
	if (mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK)
	{
		return;
	}

	switch (mcpsIndication->McpsIndication)
	{
	case MCPS_UNCONFIRMED:
	{
		break;
	}
	case MCPS_CONFIRMED:
	{
		break;
	}
	case MCPS_PROPRIETARY:
	{
		break;
	}
	case MCPS_MULTICAST:
	{
		break;
	}
	default:
		break;
	}

	// Check Multicast
	// Check Port
	// Check Datarate
	// Check FramePending
	// Check Buffer
	// Check BufferSize
	// Check Rssi
	// Check Snr
	// Check RxSlot
	if (mcpsIndication->RxData == true)
	{
		switch (mcpsIndication->Port)
		{
		default:
			break;
		}
	}
}

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] mlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm(MlmeConfirm_t *mlmeConfirm)
{
	switch (mlmeConfirm->MlmeRequest)
	{
	case MLME_JOIN:
	{
		if (mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
		{
			// Status is OK, node has joined the network
			DeviceState = DEVICE_STATE_SEND;
		}
		else
		{
			// Join was not successful. Try to join again
			DeviceState = DEVICE_STATE_JOIN;
		}
		break;
	}
	case MLME_LINK_CHECK:
	{
		if (mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
		{
		}
		break;
	}
	default:
		break;
	}
	NextTx = true;
}

LoRaMacPrimitives_t LoRaMacPrimitives;
LoRaMacCallback_t LoRaMacCallbacks;
MibRequestConfirm_t mibReq;

void LORA_Process( void )
{
	switch (DeviceState)
	{
	case DEVICE_STATE_INIT:
	{
		NRF_LOG_DEBUG("DEVICE_STATE_INIT\r\n");

		LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
		LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
		LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
		// LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
		LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks);

		TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent );

		mibReq.Type = MIB_ADR;
		mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
		LoRaMacMibSetRequestConfirm(&mibReq);

		mibReq.Type = MIB_PUBLIC_NETWORK;
		mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
		LoRaMacMibSetRequestConfirm(&mibReq);

		LoRaMacTestSetDutyCycleOn(LORAWAN_DUTYCYCLE_ON);

		LoRaMacChannelAdd(3, (ChannelParams_t)LC4);
		LoRaMacChannelAdd(4, (ChannelParams_t)LC5);
		LoRaMacChannelAdd(5, (ChannelParams_t)LC6);
		LoRaMacChannelAdd(6, (ChannelParams_t)LC7);
		LoRaMacChannelAdd(7, (ChannelParams_t)LC8);
		LoRaMacChannelAdd(8, (ChannelParams_t)LC9);
		LoRaMacChannelAdd(9, (ChannelParams_t)LC10);

		mibReq.Type = MIB_RX2_DEFAULT_CHANNEL;
		mibReq.Param.Rx2DefaultChannel = (Rx2ChannelParams_t) { 869525000, DR_3 };
		LoRaMacMibSetRequestConfirm(&mibReq);

		mibReq.Type = MIB_RX2_CHANNEL;
		mibReq.Param.Rx2Channel = (Rx2ChannelParams_t) { 869525000, DR_3 };
		LoRaMacMibSetRequestConfirm(&mibReq);
		DeviceState = DEVICE_STATE_JOIN;
		break;
	}
	case DEVICE_STATE_JOIN:
	{
		MlmeReq_t mlmeReq;
		
		NRF_LOG_DEBUG("DEVICE_STATE_JOIN\r\n");

		// Initialize LoRaMac device unique ID
		BoardGetUniqueId(DevEui);
		mlmeReq.Type = MLME_JOIN;

		mlmeReq.Req.Join.DevEui = DevEui;
		mlmeReq.Req.Join.AppEui = AppEui;
		mlmeReq.Req.Join.AppKey = AppKey;
		mlmeReq.Req.Join.NbTrials = 3;

		if (NextTx == true)
		{
			LoRaMacMlmeRequest(&mlmeReq);
		}
		DeviceState = DEVICE_STATE_SEND; // CYCLE; // DEVICE_STATE_SLEEP;
		break;
	}
	case DEVICE_STATE_SEND:
	{
		NRF_LOG_DEBUG("DEVICE_STATE_SEND\r\n");

		if (NextTx == true)
		{
			PrepareTxFrame(AppPort);

			NextTx = SendFrame();
		}
		// Schedule next packet transmission
		TxDutyCycleTime = APP_TX_DUTYCYCLE + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
		DeviceState = DEVICE_STATE_CYCLE;
		break;
	}
	case DEVICE_STATE_CYCLE:
	{
		NRF_LOG_DEBUG("DEVICE_STATE_CYCLE\r\n");

		DeviceState = DEVICE_STATE_SLEEP;

		// Schedule next packet transmission
		TimerSetValue( &TxNextPacketTimer, TxDutyCycleTime );
		TimerStart( &TxNextPacketTimer );
		break;
	}
	case DEVICE_STATE_SLEEP:
	{
		// Wake up through events
		break;
	}
	default:
	{
		DeviceState = DEVICE_STATE_INIT;
		break;
	}
	}
}
