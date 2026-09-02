#include "main.h"

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
		// xPbufQueueforautogaincontrolQueue = xQueueCreate(200, sizeof(struct pbuf*));
		xTaskCreate(udp_auto_gain_control_task, "UDP auto gain control task", 10000, NULL, AUTO_GAIN_CONTROL_TASK_PRIO, &xAutoGainControlTask);
		xMsgBuffer = xMessageBufferCreate(1024);
		xPbufQueuetcp = xQueueCreate(200, sizeof(struct pbuf*));
		xTaskCreate(tcp_parse_task, "TCP PARSE Task", THREAD_STACKSIZE, NULL, TCP_PARSE_PRIO,  NULL);
		vInitialiseTimer();
		vTaskDelete(NULL);
	}
}

void DMA_set(void *arg)
{
	DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR, (UINTPTR)0x00000010, (u32)(&dma_buffer));

	DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR, (UINTPTR)0x00000014, (u32)0x00000000);

    DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR, (UINTPTR)0x00000018, (u32)8192);

    DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR, (UINTPTR)0x0000001C, (u32)0x00000000);

}

static void hw_init_task(void *pvParameters)
{
	ProgramSi5324();
	ProgramSfpPhy();
	#ifdef XPS_BOARD_ZCU102
		IicPhyReset();
	#endif

	memset((void *)dma_buffer, 0xFF, sizeof(dma_buffer));
	Xil_DCacheFlushRange((INTPTR)dma_buffer, sizeof(dma_buffer));

	axi_gpio_objects_init_0_4_5();
	set_gpio_0_direction();
	start_control_rw_state_machine();

	XGpio_SetDataDirection(&Gpio4, 1, 0x00000000);
	XGpio_DiscreteWrite(&Gpio4, 1, (u32)0);
	XGpio_SetDataDirection(&Gpio4, 2, 0xffffffff);

    XGpio_SetDataDirection(&Gpio5, 1, 0x00000000);
	XGpio_SetDataDirection(&Gpio5, 2, 0xffffffff);

	DataPH1 = 0x00000000;
	DataPH3 = 0x00000000;
	DataPH2 = 0x00000000;
	DataPH4 = 0x00000000;

	DataAM1 = 0xffffffff;
	DataAM3 = 0xffffffff;
	DataAM2 = 0xffffffff;
	DataAM4 = 0xffffffff;

	DMA_set();

	Bram_ConfigPtr_ps_to_fpga = XBram_LookupConfig(XPAR_AXI_BRAM_CTRL_0_DEVICE_ID);
	if (Bram_ConfigPtr_ps_to_fpga == (XBram_Config *) NULL)
	    {
			xil_printf("Trouble with bram: ps-fpga");
	    }

	status = XBram_CfgInitialize(&Bram_ps_to_fpga,
			                     Bram_ConfigPtr_ps_to_fpga,
			                     Bram_ConfigPtr_ps_to_fpga->CtrlBaseAddress
								 );
	if (status != XST_SUCCESS)
	    {
			xil_printf("Trouble with bram: ps-fpga configure");
	    }

    // Bram fpga to ps
	Bram_ConfigPtr_fpga_to_ps = XBram_LookupConfig(XPAR_AXI_BRAM_CTRL_1_DEVICE_ID);
	if (Bram_ConfigPtr_fpga_to_ps == (XBram_Config *) NULL)
	    {
			xil_printf("Trouble with bram: fpga-ps");
	    }

	status = XBram_CfgInitialize(&Bram_fpga_to_ps,
			                     Bram_ConfigPtr_fpga_to_ps,
								 Bram_ConfigPtr_fpga_to_ps->CtrlBaseAddress
								 );
	if (status != XST_SUCCESS)
	    {
			xil_printf("Trouble with bram: fpga-ps don`t configure");
	    }

	vTaskDelete(NULL);
}

// void udp_auto_gain_control_task(void *pvParameters)
// {
// 	struct pbuf *p;
// 	AUTOGAINCONTROL* autogaincontrol = create_auto_gain_control_array(NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION);

// 	while (1)
// 	{
// 		if (xQueueReceive(xPbufQueueforautogaincontrolQueue, &p, portMAX_DELAY) == pdPASS && p != NULL)
// 		{
// 			auto_gain_control(p, autogaincontrol);
// 			pbuf_free(p);
// 		}
// 	}
// }

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

void check_DMA(void *arg)
{
	uint32_t strobe = 0;
	XGpioPs_WritePin(&XGpioPsInstance, 54, 1);
	vTaskDelay(pdMS_TO_TICKS(1));  // if i understand right this pause for fpga wright reaction on change pin
	strobe = XGpio_DiscreteRead(&Gpio4, 2);
	strobe = strobe & (uint32_t)1;
    XGpioPs_WritePin(&XGpioPsInstance, 54, 0);  // maybe if strobe not set - skip all task?

	XGpio_DiscreteWrite(&Gpio4, 1, (u32)1);
	DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR, (UINTPTR)0x00000000, (u32)0xC0000000);

	vTaskDelay(pdMS_TO_TICKS(1));  // here we must give pause at 0.00005 sec but can`t because 1 ms

	dummy1 = DMA_ReadReg(XPAR_S2M_0_BASEADDR, DMA_CONTROL_REG_OFFSET);
	dummy1 = dummy1 & 0x20000000;

	if (dummy1 != 0x20000000)
	{
		xil_printf("Not ended wright");
	}
	XGpio_DiscreteWrite(&Gpio4, 1, (u32)0);

	dummy_1 = DMA_ReadReg(XPAR_S2M_0_BASEADDR, DMA_CONTROL_REG_OFFSET);
    dummy_1 = dummy_1 & 0x40000000;
    if ( dummy_1 == 0 )
    	status = 0;
    else
    	status = 1;

	if (status == 1)
    {
	    DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR, (UINTPTR)0x00000000, (u32)0x26000000);  //abort cmd
		dummy_1 = 1;

		while (dummy_1 == 1)
		{
			dummy_1 = DMA_ReadReg(XPAR_S2M_0_BASEADDR, DMA_CONTROL_REG_OFFSET);
			dummy_1 = dummy_1 & 0x00400000;      // Check Abort in progress
		}

		// Set address and length  again
		DMA_set();
    }

	Xil_DCacheInvalidateRange((INTPTR) &dma_buffer, (u32) 8192);
	// next --> dma_buffer packed in udp and get out
	// maybe set flag about buffer ready?
}

int main(void)
{
	// start_cpu1();
	init_platform();
	
	Xil_ICacheEnable();
	Xil_DCacheEnable();

	xil_printf("\n Start PS-part of Zynq!\r\n");
	xil_printf("\n Version: %d.%d\n", MAJOR_SOFTWARE_VERSION, MINOR_SOFTWARE_VERSION);
	
	xTaskCreate(hw_init_task, "Hardware init", THREAD_STACKSIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(network_init_task, "Network Init", THREAD_STACKSIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(check_DMA, "DMA workspace", THREAD_LARGE_STACKSIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	vTaskStartScheduler();
	while(1);
}
