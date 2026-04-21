/*
 ============================================================================
 Name        : ttc_timer.c
 Author      :
 Version     :
 Description : TTC interrupt timer configuration
 ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"
#include "platform.h"

#include "xttcps.h"
#include "xscugic.h"
#include "ttc_timer.h"

#define tmrTIMERS_USED	3
#define tmrTIMER_0_FREQUENCY	( 5000UL )
#define tmrTIMER_1_FREQUENCY	( 5000UL )
#define tmrTIMER_2_FREQUENCY	( 4UL )

/*-----------------------------------------------------------*/
static void prvTimerHandler( void *CallBackRef );

/*-----------------------------------------------------------*/
static const BaseType_t xDeviceIDs[ tmrTIMERS_USED ] = { XPAR_XTTCPS_0_DEVICE_ID, XPAR_XTTCPS_1_DEVICE_ID, XPAR_XTTCPS_2_DEVICE_ID };
static const BaseType_t xInterruptIDs[ tmrTIMERS_USED ] = { XPAR_XTTCPS_0_INTR, XPAR_XTTCPS_1_INTR, XPAR_XTTCPS_2_INTR };

typedef struct
{
	uint32_t OutputHz;	
	uint16_t Interval;	
	uint8_t Prescaler;	
	uint16_t Options;	
} TmrCntrSetup;

static TmrCntrSetup xTimerSettings[ tmrTIMERS_USED ] =
{
	{ tmrTIMER_0_FREQUENCY, 0, 0, XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_DISABLE },
	{ tmrTIMER_1_FREQUENCY, 0, 0, XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_DISABLE },
	{ tmrTIMER_2_FREQUENCY, 0, 0, XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_DISABLE }
};

static const UBaseType_t uxInterruptPriorities[ tmrTIMERS_USED ] =
{
	configMAX_API_CALL_INTERRUPT_PRIORITY + 1,
	configMAX_API_CALL_INTERRUPT_PRIORITY,
	configMAX_API_CALL_INTERRUPT_PRIORITY - 1
};

static XTtcPs xTimerInstances[ tmrTIMERS_USED ];

/*-----------------------------------------------------------*/
void vInitialiseTimer(void)
{
	BaseType_t xStatus;
	TmrCntrSetup *pxTimerSettings;
	extern XScuGic xInterruptController;
	BaseType_t xTimer;
	XTtcPs *pxTimerInstance;
	XTtcPs_Config *pxTimerConfiguration;
	const uint8_t ucRisingEdge = 3;

	for( xTimer = 0; xTimer < 3; xTimer++ )
	{
		pxTimerInstance = &( xTimerInstances[ xTimer ] );
		pxTimerConfiguration = XTtcPs_LookupConfig( xDeviceIDs[ xTimer ] );
		configASSERT( pxTimerConfiguration );

		pxTimerSettings = &( xTimerSettings[ xTimer ] );

		xStatus = XTtcPs_CfgInitialize( pxTimerInstance, pxTimerConfiguration, pxTimerConfiguration->BaseAddress );
		if( xStatus != XST_SUCCESS )
		{
			XTtcPs_Stop( pxTimerInstance );
			xStatus = XTtcPs_CfgInitialize( pxTimerInstance, pxTimerConfiguration, pxTimerConfiguration->BaseAddress );
			configASSERT( xStatus == XST_SUCCESS );
		}

		XTtcPs_SetOptions( pxTimerInstance, pxTimerSettings->Options );

		XTtcPs_CalcIntervalFromFreq( pxTimerInstance, pxTimerSettings->OutputHz, &( pxTimerSettings->Interval ), &( pxTimerSettings->Prescaler ) );

		XTtcPs_SetInterval( pxTimerInstance, pxTimerSettings->Interval );
		XTtcPs_SetPrescaler( pxTimerInstance, pxTimerSettings->Prescaler );
		if (xTimer != 1)
		{
			XScuGic_SetPriorityTriggerType( &xInterruptController, xInterruptIDs[ xTimer ], uxInterruptPriorities[ xTimer ] << portPRIORITY_SHIFT, ucRisingEdge );

			xStatus = XScuGic_Connect( &xInterruptController, xInterruptIDs[ xTimer ], ( Xil_InterruptHandler ) prvTimerHandler, ( void * ) pxTimerInstance );
			configASSERT( xStatus == XST_SUCCESS);

			XScuGic_Enable( &xInterruptController, xInterruptIDs[ xTimer ] );

			XTtcPs_EnableInterrupts( pxTimerInstance, XTTCPS_IXR_INTERVAL_MASK );

			XTtcPs_Start( pxTimerInstance );
		}
	}
}

/*-----------------------------------------------------------*/
void start_stop_ttc_timer(uint8_t timer_ch_id, uint8_t flag_start_stop)
{
	XTtcPs *pxTimerInstance;
	pxTimerInstance = &(xTimerInstances[timer_ch_id]);
	if(flag_start_stop)
	{
		XTtcPs_ResetCounterValue(pxTimerInstance);
		XTtcPs_Start(pxTimerInstance);
	}
	else
	{
		XTtcPs_Stop(pxTimerInstance);
	}
}

/*-----------------------------------------------------------*/
uint16_t get_ttc_counter_value(uint8_t timer_ch_id)
{
	XTtcPs *pxTimerInstance;
	pxTimerInstance = &(xTimerInstances[timer_ch_id]);
	return XTtcPs_GetCounterValue(pxTimerInstance);
}
/*-----------------------------------------------------------*/
#include "task.h"
extern volatile TaskHandle_t xIrqTaskHandle;
extern u8 status_udp_sender;
u8 flag_for_led = 0;
volatile int TcpFastTmrFlag = 0;
volatile int TcpSlowTmrFlag = 0;
static void prvTimerHandler( void *pvCallBackRef )
{
	uint32_t ulInterruptStatus;
	XTtcPs *pxTimer = ( XTtcPs * ) pvCallBackRef;
	BaseType_t xHPW = pdFALSE;

	ulInterruptStatus = XTtcPs_GetInterruptStatus( pxTimer );
	XTtcPs_ClearInterruptStatus( pxTimer, ulInterruptStatus );

	configASSERT( ( XTTCPS_IXR_INTERVAL_MASK & ulInterruptStatus ) != 0 );

	if( pxTimer->Config.DeviceId == xDeviceIDs[ 0 ] )
	{
		if(status_udp_sender)
		{
			// vTaskNotifyGiveFromISR(xIrqTaskHandle, &xHPW);
			// portYIELD_FROM_ISR( xHPW );
		}
	}
	else if (pxTimer->Config.DeviceId == xDeviceIDs[ 2 ])
	{
		flag_for_led ^= 1;
		vParTestSetGPIO(LED_MIDDLE, flag_for_led);
		if (flag_for_led)
		{
			TcpSlowTmrFlag = 1;
		}
		TcpFastTmrFlag = 1;
	}
}

