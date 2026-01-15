/*
 ============================================================================
 Name        : main.h
 Author      : FET
 Version     :
 Description : Inc for platform.c
 ============================================================================
 */

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

/* Includes */
#include "xscugic.h"
#include "xscutimer.h"

/* Define */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTC_BASE_ADDR		XPAR_SCUGIC_0_CPU_BASEADDR
#define TIMER_IRPT_INTR		XPAR_SCUTIMER_INTR
#define INTC_DIST_BASE_ADDR	XPAR_SCUGIC_0_DIST_BASEADDR

/* Variables */
volatile int TcpFastTmrFlag = 0;
volatile int TcpSlowTmrFlag = 0;
static XScuTimer TimerInstance;

/*-----------------------------------------------------------*/
void start_init_all_ISR(void);
void platform_setup_interrupts(void);
void timer_callback(XScuTimer * TimerInstance);

#endif
