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
	xil_printf("\nHello bratish! I'm CPU0\r\n");
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
		xTaskCreate((TaskFunction_t)x1emacif_input_thread, "xemacif_input", THREAD_STACKSIZE, &server_netif, UDP_TASK_PRIO, NULL);
		udp_connection();
		tsp_connection_cl();
		vTaskDelete(NULL);
	}
}
/*-----------------------------------------------------------*/
void x1emacif_input_thread(void *arg)
{
    struct netif *netif = (struct netif *)arg;

    while(1)
    {
//    	if (TcpFastTmrFlag)
//    	{
//			tcp_fasttmr();
//			TcpFastTmrFlag = 0;
//		}
//		if (TcpSlowTmrFlag)
//		{
//			tcp_slowtmr();
//			TcpSlowTmrFlag = 0;
//		}
        xemacif_input(netif); /* из xadapter.h */
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
