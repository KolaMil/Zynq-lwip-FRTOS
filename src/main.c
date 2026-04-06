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
		xMsgBufferForUdp = xMessageBufferCreate(8012);
		tcp_connection_cl();
		xTaskCreate(udp_send_task, "UDP SEND Task", THREAD_STACKSIZE, NULL, UDP_TASK_PRIO,  &xIrqTaskHandle);
		vTaskSuspend(xIrqTaskHandle);
		xTaskCreate(udp_parse_task, "UDP parsing task", 3000, NULL, UDP_TASK_PRIO, NULL);
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
void udp_parse_task(void *arg)
{
    uint8_t rx_buf[3000];
	uint32_t counter_packets = 1;
	uint32_t global_counter_packets = 1;
	uint8_t *oldptr;
	uint8_t mode = 10; // todo structure of modes
	uint8_t mode_amount_deskrets;
	uint32_t len_rx_buf;
	uint32_t len_pbuf = 1;
	// oldptr = (uint8_t*) malloc(len_pbuf);
	cat240_pbuf = pbuf_alloc(PBUF_TRANSPORT, len_pbuf, PBUF_POOL);
#define TIME_OF_DAY_A_LEN 3

    while (1)
    {
        len_rx_buf = xMessageBufferReceive(xMsgBufferForUdp, rx_buf, sizeof(rx_buf), portMAX_DELAY);
        if (len_rx_buf > 0)
		{
			if (counter_packets == 1)
			{
				if (len_pbuf != len_rx_buf)
				{
					realloc(oldptr, len_rx_buf);
				}
				oldptr = (uint8_t*) malloc(len_rx_buf);
				memcpy(oldptr, rx_buf, len_rx_buf);
				mode_amount_deskrets = oldptr[30]; // 30
			}
        	else
        	{
        		//unsigned char bytes[] = {0x0, 0xa, 0x35, 0x0, 0x1, 0x2, 0xf0, 0x2f, 0x74, 0x34, 0xf, 0xff, 0x8, 0x0, 0x45, 0x0, 0x0, 0xbb, 0x34, 0xc, 0x0, 0x0, 0x80, 0x11, 0x0, 0x0, 0xc0, 0xa8, 0x1, 0x64, 0xc0, 0xa8, 0x1, 0xa, 0xd1, 0xaf, 0x13, 0x8d, 0x0, 0xa7, 0x84, 0x77, 0xf0, 0x0, 0x9f, 0xeb, 0xc8, 0x49, 0xff, 0x2, 0x0, 0x0, 0x0, 0xdd, 0x2e, 0x68, 0x2f, 0x42, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0x0, 0x5, 0x0, 0x7c, 0x0, 0x0, 0x3e, 0x1f, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x4e, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3a, 0xd2, 0x0};
        		for (uint32_t i = len_rx_buf - 2 * mode_amount_deskrets - TIME_OF_DAY_A_LEN; i < len_rx_buf - TIME_OF_DAY_A_LEN; i += 2)
        		{
        			if ((rx_buf[i] * 0x100 + rx_buf[i + 1]) > (oldptr[i] * 0x100 + oldptr[i + 1]))
        			{
        				oldptr[i] = rx_buf[i];
        				oldptr[i + 1] = rx_buf[i + 1];
        			}
        		}
        	}
        	if (counter_packets == mode)
        	{
        		oldptr[11] = global_counter_packets; //global counter ++ u8 4 * 8u 11
        		global_counter_packets++;
        		if (len_rx_buf < len_pbuf)
        		{
        			pbuf_realloc(cat240_pbuf, len_rx_buf);
					len_pbuf = len_rx_buf;
        		}
        		else if (len_rx_buf > len_pbuf)
        		{
        			cat240_pbuf = pbuf_alloc(PBUF_TRANSPORT, len_rx_buf, PBUF_POOL);
        			len_pbuf = len_rx_buf;
        		}
        		pbuf_take(cat240_pbuf, oldptr, len_rx_buf);
        		if (udp_send(udp_pcb_answer, cat240_pbuf))
        		{
					xil_printf("\n ERR of udp send \r\n");
        		}
        		free(oldptr);
        		pbuf_free(cat240_pbuf);
        		counter_packets = 0;
        	}
        	counter_packets++;
        	if (0) // Flag off udp send
        	{
        		free(oldptr);
        		pbuf_free(cat240_pbuf);
        	}
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
