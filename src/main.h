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

#include <limits.h>

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xbram.h"

/* Lwip includes. */
#include "lwip/pbuf.h"

#include "lwip_us.h"
#include "ttc_timer.h"
#include "xil_axi4_settings.h"
#include "platform.h"
#include "main_machine_functions.h"

/* Hardware defines */
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR
#define PLATFORM_ZYNQ

/* Freertos defines */
#define THREAD_STACKSIZE                1024
#define THREAD_LARGE_STACKSIZE          16384
#define TIMER_ID	                    1
#define DELAY_10_SECONDS	            10000UL
#define DELAY_1_SECOND		            1000UL
#define TIMER_CHECK_THRESHOLD	        9
#define AUTO_GAIN_CONTROL_TASK_PRIO     (tskIDLE_PRIORITY + 1)
#define UDP_TASK_PRIO                   (tskIDLE_PRIORITY + 2)
#define TCP_PARSE_PRIO                  (tskIDLE_PRIORITY + 3)
#define TCP_MSG_QUEUE_LEN               20
#define TCP_MSG_SIZE                    10

/* Software version define */
#define MAJOR_SOFTWARE_VERSION          1
#define MINOR_SOFTWARE_VERSION          0

volatile QueueHandle_t xPbufQueueudp;
volatile QueueHandle_t xPbufQueueforautogaincontrolQueue;
volatile QueueHandle_t xPbufQueuetcp;
volatile QueueHandle_t xTcpMsgQueue;
volatile QueueHandle_t xPbufQueue;
volatile QueueHandle_t xBramCmd;
volatile TaskHandle_t xIrqTaskHandle = NULL;
volatile TaskHandle_t xIrqUDPTaskHandle = NULL;
volatile TaskHandle_t xDmaTaskHadle = NULL;
volatile TaskHandle_t xTcpParseTaskHandle = NULL;
volatile MessageBufferHandle_t xMsgBuffer = NULL;
volatile TaskHandle_t xAutoGainControlTask = NULL;
volatile MessageBufferHandle_t xMsgBuffer = NULL;
volatile MessageBufferHandle_t xPacketBuffer = NULL;
auto_gain_control_t *controlPtr;

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;

// extern auto_gain_control_constants_t autogain_control_constants[20];

uint16_t counter = 0;
uint8_t counter_packets = 1;
uint8_t default_state[8192] = {0xAA, 0xBB, 0xCC};
uint8_t try_reconnect = 0;
uint8_t work_size = 10;
uint8_t old_work_size = 10;
bool extend_pack = 0;
bool starting = false;
bool update_mode = false;
int average = 0;

volatile uint32_t success_cnt;
uint32_t is_locked, is_locked_old;
uint32_t dds_gpio_ready;
uint32_t new_strobe;
uint32_t dummy1 = 0;
uint32_t ps_to_fpga_data = 0;
volatile uint16_t dma_buffer[4096] __attribute__ ((aligned (32)));
uint8_t data_from_fpga[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t radio_on_air = 0;
uint8_t status;

XBram Bram_ps_to_fpga;
XBram Bram_fpga_to_ps;
XBram_Config *Bram_ConfigPtr_fpga_to_ps;
XBram_Config *Bram_ConfigPtr_ps_to_fpga;

/*-----------------------------------------------------------*/
static void network_init_task(void *pvParameters);
static void hw_init_task(void *pvParameters);
void DMA_set(void *arg);
void vStatsTask(void *arg);
void x1emacif_input_thread(void *arg);
void check_DMA(void *arg);
void bram_work(void *arg);
void udp_parse_task(void *arg);
void udp_send_task(void *arg);
void vDmaCheckTask(void * arg);
void tcp_parse_task(void *arg);
void vTcpStatTask(void *arg);

#endif
