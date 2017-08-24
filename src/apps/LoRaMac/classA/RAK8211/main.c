#include "board.h"
#include "nrf_log_ctrl.h"

void BLE_init( void );
void LORA_Process( void );

/**
 * Main application entry point.
 */
int main(void)
{
	APP_ERROR_CHECK(NRF_LOG_INIT(NULL));

    BoardInitMcu( );

	BLE_init( );

	for (;;)
	{
		while (NRF_LOG_PROCESS())
			;
		LORA_Process( );
	}
}
