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
		tcp_connection_cl();
		xPbufQueueudp = xQueueCreate(200, sizeof(struct pbuf*));
		xTaskCreate(udp_parse_task, "UDP parsing task", 10000, NULL, UDP_TASK_PRIO, NULL);
		xPbufQueueforautogaincontrolQueue = xQueueCreate(200, sizeof(struct pbuf*));
		xTaskCreate(udp_auto_gain_control_task, "UDP auto gain control task", 10000, NULL, AUTO_GAIN_CONTROL_TASK_PRIO, &xAutoGainControlTask);
		xMsgBuffer = xMessageBufferCreate(1024);
		xPbufQueuetcp = xQueueCreate(200, sizeof(struct pbuf*));
		xTaskCreate(tcp_parse_task, "TCP PARSE Task", THREAD_STACKSIZE, NULL, TCP_PARSE_PRIO,  NULL);
		vInitialiseTimer();
		vTaskDelete(NULL);
	}
}

/*-----------------------------------------------------------*/
void udp_auto_gain_control_task(void *pvParameters)
{
	struct pbuf *p;
	AUTOGAINCONTROL* autogaincontrol = create_auto_gain_control_array(NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION);

	while (1)
	{
		if (xQueueReceive(xPbufQueueforautogaincontrolQueue, &p, portMAX_DELAY) == pdPASS && p != NULL)
		{
			auto_gain_control(p, autogaincontrol);
			pbuf_free(p);
		}
	}
}

/*-----------------------------------------------------------*/
void udp_parse_task(void *pvParameters)
{
	struct pbuf *p;
	struct pbuf *old_pbuf = NULL;
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint8_t counter_of_packets = 1;
	uint8_t nominal_counter_value = 10;
	uint8_t *curr_data;
	uint8_t *o_curr_data;

	while (1)
	{
		if (xQueueReceive(xPbufQueueudp, &p, portMAX_DELAY) == pdPASS && p != NULL)
		{
			if (counter_of_packets == 1)
			{
				old_pbuf = p;
				o_curr_data = (uint8_t *)old_pbuf->payload;
			}
			else
			{
				curr_data = (uint8_t *)p->payload;
				static uint8_t iw;
				static uint32_t predel_na_shag;
				static uint32_t total_len;
				static uint8_t kursor;
				total_len = p->tot_len;
				kursor = 32;
				for (struct pbuf *q = p, *ref = old_pbuf; q != NULL; q = q->next, ref = ref->next)
				{
					if (total_len > 1480)
					{
						total_len -= 1480;
						predel_na_shag = 1480;
					}
					else
					{
						predel_na_shag = total_len % 1480;
					}
					for (uint32_t i = 0; i < predel_na_shag; i += 2)
					{
						if ((((uint8_t *)q->payload + kursor)[i] > ((uint8_t *)ref->payload + kursor)[i]) || (((((uint8_t *)q->payload + kursor)[i] == ((uint8_t *)ref->payload + kursor)[i]) && (((uint8_t *)q->payload + kursor)[i + 1] > ((uint8_t *)ref->payload + kursor)[i + 1]))))
						{
							((uint8_t *)ref->payload + kursor)[i] = ((uint8_t *)q->payload + kursor)[i];
							((uint8_t *)ref->payload + kursor)[i + 1] = ((uint8_t *)q->payload + kursor)[i + 1];
						}
					}
					kursor = 0;
				}
				pbuf_free(p);
				if (counter_of_packets >= nominal_counter_value)
				{
					if (eTaskGetState(xAutoGainControlTask) != eSuspended)
					{
						if (xQueueSend(xPbufQueueforautogaincontrolQueue, &old_pbuf, xHigherPriorityTaskWoken) != pdPASS)
						{
							pbuf_free(old_pbuf);
						}
						udp_send(udp_pcb_answer, old_pbuf);
					}
					if (eTaskGetState(xAutoGainControlTask) == eSuspended)
					{
						udp_send(udp_pcb_answer, old_pbuf);
						pbuf_free(old_pbuf);
					}
					counter_of_packets = 0;
				}
			}
			counter_of_packets++;
		}
	}
}

/*-----------------------------------------------------------*/
void tcp_parse_task(void *arg)
{
	struct pbuf *p;

	while(1)
	{
		if (xQueueReceive(xPbufQueuetcp, &p, portMAX_DELAY) == pdPASS && p != NULL)
		{
			parse_msg(p);
			pbuf_free(p);
		}
	}
}

/*-----------------------------------------------------------*/
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
