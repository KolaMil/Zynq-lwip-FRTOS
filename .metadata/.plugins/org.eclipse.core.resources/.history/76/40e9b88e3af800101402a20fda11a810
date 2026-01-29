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

/* Axi dma */
#include "xparameters.h"
//#include "xaxidma.h"

/* Platform variables */
extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;

/* Platform includes */
#include "platform.h"
#include "IntQueueTimer.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"

/* Dual core includes. */
#include "cpu1_init.h"

/* Lwip includes. */
#include "lwip.h"
#include "lwip/pbuf.h"

/* Data Lwip */
static const char data_imit_dma[8192] = "Static Data to Send";

//extern XGpioPs  psGpioInstancePtr;
volatile TaskHandle_t xIrqTaskHandle = NULL;
volatile TaskHandle_t xTcpParseTaskHandle = NULL;

/* Freertos defines */
#define THREAD_STACKSIZE 1024
#define TIMER_ID	1
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
#define TIMER_CHECK_THRESHOLD	9
#define UDP_TASK_PRIO (tskIDLE_PRIORITY + 2)
#define TCP_PARSE_PRIO (tskIDLE_PRIORITY + 2)
#define TCP_MSG_QUEUE_LEN 10
#define TCP_MSG_SIZE      sizeof(uint8_t)

//extern XGpioPs  psGpioInstancePtr;
volatile QueueHandle_t xTcpMsgQueue;

/* Data to send */
uint8_t default_state[8192] = {0xAA, 0xBB, 0xCC};

/*-----------------------------------------------------------*/
static void network_init_task(void *pvParameters);
void vStatsTask(void *arg);
void x1emacif_input_thread(void *arg);
void udp_send_task(void *arg);
void tcp_parse_task(void *arg);

#endif
