/*
 ============================================================================
 Name        : main.h
 Author      : FET
 Version     :
 Description : Inc for main
 ============================================================================
 */

#ifndef __MAIN_H_
#define __MAIN_H_

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xaxidma.h"

/* Dual core includes. */
#include "cpu1_init.h"

/* Lwip includes. */
#include "lwip.h"

/* Freertos defines */
#define THREAD_STACKSIZE 512
#define TIMER_ID	1
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
#define TIMER_CHECK_THRESHOLD	9
#define UDP_TASK_PRIO (tskIDLE_PRIORITY + 2)

/*-----------------------------------------------------------*/
static void network_init_task(void *pvParameters);
void x1emacif_input_thread(void *arg);

#endif
