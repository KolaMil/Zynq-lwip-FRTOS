/*
 ============================================================================
 Name        : platform.h
 Author      :
 Version     :
 Description : Inc for platform.c
 ============================================================================
 */

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

/* Includes */
#include "xscugic.h"
#include "xscutimer.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "FreeRTOS.h"
#include "xparameters.h"
#include "xgpiops.h"

#define TIMER_DEVICE_ID		        XPAR_SCUTIMER_DEVICE_ID
#define INTC_DEVICE_ID		        XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTC_BASE_ADDR		        XPAR_SCUGIC_0_CPU_BASEADDR
#define TIMER_IRPT_INTR		        XPAR_SCUTIMER_INTR
#define INTC_DIST_BASE_ADDR	        XPAR_SCUGIC_0_DIST_BASEADDR
#define partstGPIO_54_OUTPUT		( 54 )
#define partstGPIO_55_OUTPUT		( 55 )
#define partstGPIO_56_OUTPUT		( 56 )
#define partstGPIO_57_OUTPUT		( 57 )
#define partstGPIO_58_OUTPUT		( 58 )
#define IO_L24N_T3_12               0
#define IO_L23P_T3_12               1
#define IO_L23N_T3_12               2
#define IO_L19P_T3_12               3
#define IO_L19N_T3_12               4
#define LED_LEFT                    8
#define LED_MIDDLE                  7
#define LED_RIGHT                   6

#define partstDIRECTION_OUTPUT	    ( 1 )
#define partstOUTPUT_ENABLED	    ( 1 )

XScuGic xInterruptController;

/*-----------------------------------------------------------*/
void init_platform(void);
void prvSetupHardware(void);
void vParTestInitialise(void);
void vParTestSetGPIO(UBaseType_t uxPIN, BaseType_t xValue);

#endif
