/*
 ============================================================================
 Name        : main.c
 Author      : FET
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
	xTaskCreate(network_init_task, "Network Init", THREAD_STACKSIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(vStatsTask, "StatsTask", THREAD_STACKSIZE, NULL, UDP_TASK_PRIO, NULL);
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
		xTaskCreate((TaskFunction_t)x1emacif_input_thread, "xemacif_input", THREAD_STACKSIZE, &server_netif, UDP_TASK_PRIO, NULL);
		udp_connection(default_state, sizeof(default_state));
		tcp_connection_cl();
		xTaskCreate(udp_send_task, "UDP SEND Task", THREAD_STACKSIZE, NULL, UDP_TASK_PRIO,  &xIrqTaskHandle);
		vTaskSuspend(xIrqTaskHandle);
		xTcpMsgQueue = xQueueCreate(TCP_MSG_QUEUE_LEN, sizeof(void*));
		xMsgBuffer = xMessageBufferCreate(1024);
		xTaskCreate(tcp_parse_task, "TCP PARSE Task", THREAD_STACKSIZE, NULL, TCP_PARSE_PRIO,  &xTcpParseTaskHandle);
		vInitialiseTimer();
		vTaskDelete(NULL);
	}
}

uint16_t counter = 0;
int average = 0;
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
		vParTestSetGPIO(IO_L23P_T3_12, flag);
    }
}

/*-----------------------------------------------------------*/
void tcp_parse_task(void *arg)
{
	uint8_t payload[7];
	while(1)
	{
		// if (xQueueReceive(xTcpMsgQueue, &payload, portMAX_DELAY) == pdPASS)
		// {
		// 	xil_printf("\nSize of bytes %02x\n", payload[6]);
			 
		// 	parse_msg(payload, 7);
		// }
		if (xMessageBufferReceive(xMsgBuffer, payload, 7, portMAX_DELAY))
		{
			xil_printf("\nSize of bytes %02x\n", payload[6]);
			parse_msg(payload, payload[6]);
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
    }
}

/*-----------------------------------------------------------*/
void x1emacif_input_thread(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    while(1)
    {
        xemacif_input(netif);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
