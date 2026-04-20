/*
 ============================================================================
 Name        : main.h
 Author      :
 Version     :
 Description : Inc for main
 ============================================================================
 */

#ifndef __MAIN_H_
#define __MAIN_H_

/* Axi dma */
#include "xparameters.h"
//#include "xaxidma.h"

/* Platform includes */
#include "platform.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "message_buffer.h"
#include "stream_buffer.h"


/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"

/* Dual core includes. */
#include "cpu1_init.h"

/* Lwip includes. */
#include "lwip/pbuf.h"

#include "iwip.h"
#include "ttc_timer.h"

/* Auto gain control */
#include "auto_gain_control.h"

/* Freertos defines */
#define THREAD_STACKSIZE        1024
#define TIMER_ID	            1
#define DELAY_10_SECONDS	    10000UL
#define DELAY_1_SECOND		    1000UL
#define TIMER_CHECK_THRESHOLD	9
#define UDP_TASK_PRIO           (tskIDLE_PRIORITY + 2)
#define TCP_PARSE_PRIO          (tskIDLE_PRIORITY + 3)
#define TCP_MSG_QUEUE_LEN       20
#define TCP_MSG_SIZE            10

/* Software version define */
#define MAJOR_SOFTWARE_VERSION  0
#define MINOR_SOFTWARE_VERSION  2

volatile QueueHandle_t xTcpMsgQueue;
volatile QueueHandle_t xPbufQueue;
volatile MessageBufferHandle_t xMsgBuffer = NULL;

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;

uint8_t default_state[8192] = {0xAA, 0xBB, 0xCC};
uint8_t extend_pack = 0;
uint8_t try_reconnect = 0;
bool mode = true;

/*-----------------------------------------------------------*/
static void network_init_task(void *pvParameters);
void vStatsTask(void *arg);
void x1emacif_input_thread(void *arg);
void udp_parse_task(void *arg);
void udp_send_task(void *arg);
void tcp_parse_task(void *arg);
void vTcpStatTask(void *arg);

#endif
