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
		xMsgBuffer = xMessageBufferCreate(1024);
		xPbufQueue = xQueueCreate(200, sizeof(struct pbuf*));
		xTaskCreate(udp_parse_task, "UDP parsing task", 10000, NULL, UDP_TASK_PRIO, NULL);
		xPbufforautogaincontrolQueue = xQueueCreate(200, sizeof(struct pbuf*));
		xTaskCreate(udp_auto_gain_control_task, "UDP auto gain control task", 10000, NULL, AUTO_GAIN_CONTROL_TASK_PRIO, NULL);
		xTaskCreate(tcp_parse_task, "TCP PARSE Task", THREAD_STACKSIZE, NULL, TCP_PARSE_PRIO,  NULL);
		vInitialiseTimer();
		vTaskDelete(NULL);
	}
}

/*-----------------------------------------------------------*/
void udp_auto_gain_control_task(void *pvParameters)
{
	struct pbuf *p;
	struct pbuf *old_pbuf = NULL;
	uint8_t counter_of_packets = 0;
	uint8_t nominal_counter_value = 10;
	AUTOGAINCONTROL* autogaincontrol = create_auto_gain_control_array(NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION);
	uint16_t start_azimuth_ = 0;
	uint16_t end_azimuth;
	uint8_t *curr_data;
	uint32_t couner_;
	uint8_t *samples_;
	uint32_t size_of_samples;

	while (1)
	{
		if (xQueueReceive(xPbufforautogaincontrolQueue, &p, portMAX_DELAY) == pdPASS && p != NULL)
		{
			curr_data = (uint8_t*)p->payload;
			start_azimuth_ = curr_data[12] * 0x100 + curr_data[13];
			end_azimuth   = curr_data[14] * 0x100 + curr_data[15];
			couner_ = curr_data[28] * 0x10000 + curr_data[29] * 0x100 + curr_data[30];
			size_of_samples = couner_ * 2;
			samples_ = (uint8_t *)(curr_data + 32);
			auto_gain_control(start_azimuth_, end_azimuth, samples_, size_of_samples, autogaincontrol);
			if (counter_of_packets >= nominal_counter_value)
			{
				udp_send(udp_pcb_answer, old_pbuf);
				pbuf_free(old_pbuf);
				old_pbuf = NULL;
				counter_of_packets = 0;
			}
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
	uint16_t start_azimuth_ = 0;
	uint16_t end_azimuth;
	uint32_t couner_;
	uint32_t size_of_samples;
	uint8_t *old_samples_;
	uint8_t *new_samples_;
	uint8_t *curr_data;
	uint8_t *o_curr_data;

	while (1)
	{
		if (xQueueReceive(xPbufQueue, &p, portMAX_DELAY) == pdPASS && p != NULL)
		{
			if (counter_of_packets == 1)
			{
				old_pbuf = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RAM);
				if (old_pbuf)
				{
					pbuf_copy(old_pbuf, p);
				}
				pbuf_free(p);
				o_curr_data = (uint8_t *)old_pbuf->payload;
				start_azimuth_ = o_curr_data[12] * 0x100 + o_curr_data[13];
				end_azimuth = o_curr_data[14] * 0x100 + o_curr_data[15];
				couner_ = o_curr_data[28] * 0x10000 + o_curr_data[29] * 0x100 + o_curr_data[30];
				size_of_samples = couner_ * 2;
				old_samples_ = (uint8_t *)(o_curr_data + 32);
			}
			else
			{
				curr_data = (uint8_t *)p->payload;
				start_azimuth_ = curr_data[12] * 0x100 + curr_data[13];
				end_azimuth = curr_data[14] * 0x100 + curr_data[15];
				couner_ = curr_data[28] * 0x10000 + curr_data[29] * 0x100 + curr_data[30];
				size_of_samples = couner_ * 2;
				new_samples_ = (uint8_t *)(curr_data + 32);
				for (uint32_t i = 0; i < size_of_samples; i += 2)
				{
					if (new_samples_[i] > old_samples_[i] || ((new_samples_[i] == old_samples_[i]) && (new_samples_[i + 1] > old_samples_[i + 1])))
					{
						old_samples_[i] = new_samples_[i];
						old_samples_[i + 1] = new_samples_[i + 1];
					}
				}
				pbuf_free(p);
				if (counter_of_packets >= nominal_counter_value)
				{
					udp_send(udp_pcb_answer, old_pbuf);
					if (xQueueSend(xPbufforautogaincontrolQueue, &old_pbuf, xHigherPriorityTaskWoken) != pdPASS)
					{
						pbuf_free(p);
					}
					counter_of_packets = 0;
					pbuf_free(old_pbuf);
				}
			}
			counter_of_packets++;
		}
	}
}

/*-----------------------------------------------------------*/
void tcp_parse_task(void *arg)
{
	uint8_t payload[10];
	size_t size;
	tcp_command cmd;

	while(1)
	{
		size = xMessageBufferReceive(xMsgBuffer, payload, sizeof(payload), portMAX_DELAY);
		if (size > 0)
		{
			parse_msg(payload, size);
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
