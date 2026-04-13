/*
 ============================================================================
 Name        : main.c
 Author      :
 Version     :
 Description : FreeRTOS with LwIp and AxIdma
 ============================================================================
 */

#include "main.h"

int main(void)
{
	start_cpu1();
	init_platform();
	xil_printf("\n Start PS-part of Zynq!\r\n");
	xil_printf("\n Version: %d.%d\n", MAJOR_SOFTWARE_VERSION, MINOR_SOFTWARE_VERSION);
	xTaskCreate(network_init_task, "Network Init", THREAD_STACKSIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(vStatsTask, "StatsTask", THREAD_STACKSIZE, NULL, UDP_TASK_PRIO - 1, NULL);
	vTaskStartScheduler();

	while(1);
}

/*-----------------------------------------------------------*/
static void network_init_task(void *pvParameters)
{
	const TickType_t x1second = pdMS_TO_TICKS(DELAY_1_SECOND);
	while(1)
	{
		vTaskDelay(x1second);
		lwip_network_setup();
		xTaskCreate((TaskFunction_t)x1emacif_input_thread, "xemacif_input", THREAD_STACKSIZE, &server_netif, tskIDLE_PRIORITY, NULL);
		udp_connection(default_state, sizeof(default_state));
		xMsgBuffer = xMessageBufferCreate(1024);
		// xMsgBufferForUdp = xMessageBufferCreate(8012);
		tcp_connection_cl();
		xTaskCreate(udp_send_task, "UDP SEND Task", THREAD_STACKSIZE, NULL, UDP_TASK_PRIO,  &xIrqTaskHandle);
		vTaskSuspend(xIrqTaskHandle);
		xPbufQueue = xQueueCreate(200, sizeof(struct pbuf*));
		xTaskCreate(udp_parse_task, "UDP parsing task", 4096, NULL, UDP_TASK_PRIO, NULL);
		xTcpMsgQueue = xQueueCreate(TCP_MSG_QUEUE_LEN, sizeof(void*));
		xTaskCreate(tcp_parse_task, "TCP PARSE Task", THREAD_STACKSIZE, NULL, TCP_PARSE_PRIO,  &xTcpParseTaskHandle);
		xTaskCreate(vTcpStatTask, "TCP monitoring task", THREAD_STACKSIZE, NULL, tskIDLE_PRIORITY, NULL);
		vInitialiseTimer();
		vTaskDelete(NULL);
	}
}

/*-----------------------------------------------------------*/
void udp_send_task(void *arg)
{
    xil_printf("UDP task started (simple delay 2ms)\r\n");
    u8 flag = 0;

    while(1)
    {
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		flag ^= 1;
		vParTestSetGPIO(IO_L24N_T3_12, flag);
		start_stop_ttc_timer(TTC_TIMER_CHANNEL_2, 1);
		counter = get_ttc_counter_value(TTC_TIMER_CHANNEL_2);
		udp_package_send();
		counter = get_ttc_counter_value(TTC_TIMER_CHANNEL_2) - counter;
		if (average != 0)
		{
			average = (average + counter) / 2;
		}
		else
		{
			average = counter;
		}
    	vParTestSetGPIO(IO_L24N_T3_12, flag);
		vParTestSetGPIO(IO_L23P_T3_12, flag);
    	vParTestSetGPIO(IO_L23N_T3_12, flag);
    	vParTestSetGPIO(IO_L19P_T3_12, flag);
    }
}

/*-----------------------------------------------------------*/
void udp_parse_task(void *pvParameters) {
	struct pbuf *p;
	struct pbuf *q;
	uint32_t total_bytes = 0;

	while (1)
	{
		if (xQueueReceive(xPbufQueue, &p, portMAX_DELAY) == pdPASS && p != NULL)
		{
			for(q = p; q != NULL; q = q->next)
			{

			}
			total_bytes += p->len;
			// xil_printf("\nLen %u\n", total_bytes);
			err_t err = udp_send(udp_pcb_answer, p);
			pbuf_free(p);
			total_bytes = 0;
		}
	}
}

/*-----------------------------------------------------------*/
#include "projdefs.h"
void tcp_parse_task(void *arg)
{
	uint8_t payload[10];
	size_t size;
	while(1)
	{
		size = xMessageBufferReceive(xMsgBuffer, payload, sizeof(payload), portMAX_DELAY);
		if (size > 0)
		{
			xil_printf("\nSize of bytes %02x\n", size);
			parse_msg(payload, size);
		}
	}
}

/*-----------------------------------------------------------*/
void vStatsTask(void *arg)
{
    start_stop_ttc_timer(TTC_TIMER_CHANNEL_2, 1);
	const TickType_t xPeriod = pdMS_TO_TICKS(5000);
	u8 flag_for_led = 0;
    while(1)
    {
		vParTestSetGPIO(LED_LEFT, flag_for_led);
		flag_for_led ^= 1;
		vTaskDelay(xPeriod);
        if (average % 100 > 10)
		{
        	xil_printf("Average packet sending time via UDP %u,%u\r\n", average * 900 / 100000, average % 100);
		}
		else
		{
			xil_printf("Average packet sending time via UDP %u,0%u\r\n", average * 900 / 100000, average % 100);
		}
		average = 0;
		
		need_reconnect(tcp_pcb);
		if (try_reconnect)
		{
			reconnection_tcp(tcp_pcb);
		}
    }
}

/*-----------------------------------------------------------*/
void vTcpStatTask(void *arg)
{
    xil_printf("Task monitoring tcp");
	const TickType_t monitor_interval = pdMS_TO_TICKS(30000);
	while (1)
	{
		update_monitor(monitor);
		print_status(monitor);
		
		vTaskDelay(monitor_interval);
	}
}

/*-----------------------------------------------------------*/
#include "netif\xadapter.h"
#include "lwip\priv\tcp_priv.h"
void x1emacif_input_thread(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    while(1)
    {
        if(TcpFastTmrFlag)
		{
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag)
		{
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(netif);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
