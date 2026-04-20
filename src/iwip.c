/*
 ============================================================================
 Name        : lwip.c
 Author      :
 Version     :
 Description : Init api lwip with udp&tcp
 ============================================================================
 */

#include "iwip.h"

/*-----------------------------------------------------------*/
void lwip_network_setup(void)
{
    ip_addr_t ipaddr, netmask, gw;
    unsigned char mac[] = {0x00,0x0A,0x35,0x00,0x01,0x02};

    lwip_init();
	inet_aton(LOCAL_IP_ADDRESS, &ipaddr);
	inet_aton(LOCAL_NET_MASK, &netmask);
	inet_aton(LOCAL_GATEWAY, &gw);

    if (!xemac_add(&server_netif, &ipaddr, &netmask, &gw, mac, XPAR_XEMACPS_0_BASEADDR))
    {
        xil_printf(" Error adding N/W interface\r\n");
        vTaskDelete(NULL);
        return;
    }
    netif_set_default(&server_netif);
    netif_set_up(&server_netif);
}

/*-----------------------------------------------------------*/
err_t udp_connection(uint8_t *data, size_t size)
{
	udp_pcb_conn = udp_new();
	udp_pcb_answer = udp_new();
	if(!udp_pcb_conn)
	{
		xil_printf(" Cannot create UDP PCB\r\n");
		vTaskDelete(NULL);
		return ERR_MEM;
	}
	ip_addr_t remote_ip;
	inet_aton(REMOTE_IP_ADDRESS, &remote_ip);
	sender = add_sender(udp_pcb_conn, data, size);
	if (sender == NULL)
	{
		vTaskDelete(NULL);
		return ERR_RTE;
	}
//	udp_connect(udp_pcb_conn, &remote_ip, UDP_REMOTE_PORT);
	udp_connect(udp_pcb_answer, &remote_ip, UDP_REMOTE_PORT);
	udp_bind(udp_pcb_conn, IP_ADDR_ANY, 5005);
	udp_recv(udp_pcb_conn, udp_receive_callback, NULL);
	return ERR_OK;
}

/*-----------------------------------------------------------*/
void compose(uint8_t* src, cat240_storage_t* store)
{
	memcpy(src, store->header_data, store->header_size);
	memcpy(src + store->header_size, store->video_data, store->data_size);
	memcpy(src + store->header_size + store->data_size, store->time_per_bytes, store->time_size);
}

/*-----------------------------------------------------------*/
void set_mode()
{
}

/*-----------------------------------------------------------*/
void refactor(cat240_message_t* src, cat240_storage_t* dir)
{
	uint8_t avg_az = (src->header_nano.start_az + dir->start_az[0]) / 2;
	*dir->end_az = avg_az + 0x00DA;
	dir->start_az[0] = avg_az;
	for (size_t i = 0; i < dir->data_size; i++)
	{
		if (src->video_block.data[i] > dir->video_data[i])
		{
			dir->video_data[i] = src->video_block.data[i];
		}
	}
}

/*-----------------------------------------------------------*/
void refactor_more(cat240_storage_t* lst, cat240_storage_t** dir, size_t size)  // lst is a pointer of final struct, dir is a pointer of massive structers 
{
	if (!size)
	{
		return;
	}
	// size_t _size = dir[0]->data_size;
	size_t _size = 124;
	uint64_t max[124] = {0};
	
	for (size_t i = 0; i < size; i++)
	{
		for (size_t j = 0; j < _size; j++)
		{
			if (dir[i]->video_data[j] > max[j])
			{
				max[j] = dir[i]->video_data[j];
			}
		}
	}
	for (size_t i = 0; i < _size; i++)
	{
		lst->header_data[i] = (double)max[i];
	}
}

/*-----------------------------------------------------------*/
void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	if (p == NULL) return;
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if (xQueueSendFromISR(xPbufQueue, &p, &xHigherPriorityTaskWoken) != pdPASS)
	{
		pbuf_free(p);
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*-----------------------------------------------------------*/
err_t udp_package_send(void)
{
	err_t err = send_udp_data(sender);
	return err;
}

/*-----------------------------------------------------------*/
volatile u8 stat_tcp_con = 0;
err_t tcp_connection_cl(void)
{
	err_t err;
	ip_addr_t remote_addr;

	err = inet_aton(REMOTE_IP_ADDRESS, &remote_addr);

	tcp_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!tcp_pcb)
	{
		xil_printf(" Error creating PCB. Out of Memory\n\r");
		return ERR_MEM;
	}

	monitor = create_tcp_monitor(tcp_pcb);
	update_monitor(monitor);

	xil_printf(" Try to connect");
	err = tcp_connect(tcp_pcb, &remote_addr, TCP_REMOTE_PORT, client_connected);
	vTaskDelay(pdMS_TO_TICKS(2000)); // need for callback-function client_connected true work
	if (err)
	{
		xil_printf(" Error on tcp_connect: %d\r\n", err);
		tcp_client_close(tcp_pcb);
		return ERR_CLSD;
	}
	return ERR_OK;
}

/*-----------------------------------------------------------*/
err_t need_reconnect(struct tcp_pcb *pcb)
{
	xil_printf(" Checking status TCP...\n");
	update_monitor(monitor);
	xil_printf("%s\n", tcp_state_to_string(monitor->state));
	if (monitor->pcb->state != ESTABLISHED)
	{
		try_reconnect = 1;
	}
	else{ try_reconnect = 0; }
	return ERR_OK;
}

/*-----------------------------------------------------------*/
err_t reconnection_tcp(struct tcp_pcb *pcb)
{
	xil_printf(" Closed TCP-connection. Retry... \n");
	xil_printf(
			"%d", try_reconnect);
	if (pcb->state == ESTABLISHED)
	{
		try_reconnect = 0;
		return ERR_ALREADY;
	}
	else
	{
		xil_printf(" Rebuild TCP stack for new try to connect... \n"); // need a timer for rebuild
		destroy_monitor(monitor);
		tcp_sent(pcb, NULL);
		tcp_err(pcb, NULL);
		tcp_poll(pcb, NULL, 0);
		tcp_abort(pcb);
		tcp_connection_cl();
	}
	return ERR_OK;
}

/*-----------------------------------------------------------*/
err_t tcp_client_close(struct tcp_pcb *pcb)
{
	err_t err;

	if (pcb != NULL) {
		tcp_sent(pcb, NULL);
		tcp_err(pcb, NULL);
		tcp_poll(pcb, NULL, 0);
		err = tcp_close(pcb);
		if (err != ERR_OK) {
			tcp_abort(pcb);
		}
	}
	return err;
}

/*-----------------------------------------------------------*/
err_t client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    if (err != ERR_OK) {
        xil_printf(" Connection failed with error code: %d\n\r", err);
        return err;
    }
    xil_printf("\n Successfully connected to server\n\r");
    stat_tcp_con = 1;
	tcp_recv(tpcb, recv_callback);
	tcp_poll(tpcb, need_reconnect, 20);
    return err;
}

/*-----------------------------------------------------------*/
uint8_t get_tetrada(uint8_t *data, size_t len) {
    if (data == NULL || len == 0) {
        return 0;
    }
	uint8_t lowest_byte = data[len - 1];
    
    return (lowest_byte >> 4) & 0x0F;
}

/*-----------------------------------------------------------*/
volatile u8 flag_tcp = 0;
volatile u8 status_udp_sender = 0;
int parse_msg(void* p, size_t size)
{
	uint8_t *byte_ptr = (uint8_t*) p;
	uint8_t last_byte = byte_ptr[size - 1];
	xil_printf("getting size %02x", size);
	if (!extend_pack)
	{
		uint8_t tetrada = 0x0C;
		tetrada = get_tetrada(byte_ptr, size);
		// xil_printf("Tetrada of command %02x", tetrada);  FOR DEBUG!!!
		switch (tetrada)
		{
		case 0x0E:
			xil_printf("TOB VALUE");
			monitor->last_recv_cmd = CMD_TOB_VALUE_REQS;
			break;
		
		case 0x0F:
			xil_printf("WORKTYPES");
			monitor->last_recv_cmd = CMD_WORKTYPE_REQS;
			break;

		case 0x0D:
			xil_printf("TOB POINT");
			monitor->last_recv_cmd = CMD_TOB_POINT;
			break;

		case 0x0C:
			xil_printf("Command with C tetrada");
			if (last_byte == CMD_STOP)
			{
				xil_printf("Stop UDP send!");
				status_udp_sender = 0;
				monitor->last_recv_cmd = CMD_STOP;
			}
			else if (last_byte == CMD_START)
			{
				xil_printf("Start UDP send!");
				status_udp_sender = 1;
				// vTaskResume(xIrqTaskHandle);
				monitor->last_recv_cmd = CMD_START;
			}
			else if (last_byte == CMD_WORKTYPE_SET)
			{
				xil_printf("Set worktype!");
				mode = *byte_ptr;  // make a change mode here!
				monitor->last_recv_cmd = CMD_WORKTYPE_SET;
			}
			else if (last_byte == CMD_CTRL_AMPL)
			{
				xil_printf("Set amplifyer control mode!");
				monitor->last_recv_cmd = CMD_CTRL_AMPL;
			}
			else if (last_byte == CMD_SEA_FILTER)
			{
				xil_printf("Set sea filter!");
				monitor->last_recv_cmd = CMD_SEA_FILTER;
			}
			else if (last_byte == CMD_PREC_FILTER)
			{
				xil_printf("Set rain filter!");
				monitor->last_recv_cmd = CMD_PREC_FILTER;
			}
			else if (last_byte == CMD_VELOCITY)
			{
				xil_printf("Set velocity!");
				monitor->last_recv_cmd = CMD_VELOCITY;
			}
			else if (last_byte == CMD_FREQ_CHGE)
			{
				xil_printf("Set frequecny!");
				monitor->last_recv_cmd = CMD_FREQ_CHGE;
			}
			else if (last_byte == CMD_TELEMETRY_REQS)
			{
				xil_printf("Get a telemetry!");
				monitor->last_recv_cmd = CMD_TELEMETRY_REQS;
			}
			else
			{
				xil_printf("Reset faults!");
				monitor->last_recv_cmd = CMD_RESET_FAULTS;
			}
			break;

		default:
			xil_printf("Command not found!");
			break;
		}
	}
	else{
		xil_printf("Make a blind sector! \n");
		// massive with data send to PL-part in BRAM
		// coming soon
		monitor->last_recv_cmd = CMD_BLANK_SEC;
	}
	return 0;
}

/*-----------------------------------------------------------*/
uint16_t ssize;
err_t recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	if (!p)
	{
		tcp_close(tpcb);
		tcp_recv(tpcb, NULL);
		return ERR_VAL;
	}
	extend_pack = 0;
	tcp_recved(tpcb, p->len);

	uint8_t buf[6] = {0};
	if (p->len == 6){ extend_pack = 1; }

	memcpy(buf, p->payload, 6);
	ssize = p->len;
	xMessageBufferSendFromISR(xMsgBuffer, (void*)buf, ssize, NULL);
	if (tcp_sndbuf(tpcb) > p->len)
	{
		err = tcp_write(tpcb, p->payload, p->len, 1);
	}
	else{ xil_printf("no space in tcp_sndbuf\n\r"); }

	flag_tcp ^= 1;
	pbuf_free(p);

	return ERR_OK;
}
