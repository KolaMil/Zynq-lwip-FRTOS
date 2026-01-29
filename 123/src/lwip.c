/*
 ============================================================================
 Name        : lwip.c
 Author      : FET
 Version     :
 Description : Init socet api lwip with
 ============================================================================
 */

#include "lwip.h"

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
        xil_printf("Error adding N/W interface\r\n");
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
	if(!udp_pcb_conn)
	{
		xil_printf("Cannot create UDP PCB\r\n");
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
	udp_connect(udp_pcb_conn, &remote_ip, UDP_REMOTE_PORT);
	return ERR_OK;
}

/*-----------------------------------------------------------*/
err_t udp_package_send(void)
{
	err_t err = send_udp_data(sender);
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
		xil_printf("Error creating PCB. Out of Memory\n\r");
		return;
	}

	monitor = create_tcp_monitor(tcp_pcb);
	update_monitor(monitor);

	xil_printf("Try to connect");
	err = tcp_connect(tcp_pcb, &remote_addr, TCP_REMOTE_PORT, client_connected);
	vTaskDelay(pdMS_TO_TICKS(2000)); // need for callback-function client_connected true work
	if (err)
	{
		xil_printf("Error on tcp_connect: %d\r\n", err);
		tcp_client_close(tcp_pcb);
		return ERR_CLSD;
	}
}

err_t need_reconnect(struct tcp_pcb *pcb)
{
	if (pcb->state != ESTABLISHED)
	{
		try_reconnect = 1;
	}
	else{ try_reconnect = 0; }
	return ERR_OK;
}

err_t reconnection_tcp(struct tcp_pcb *pcb)
{
	xil_printf(" Closed TCP-connection. Retry... \n");
	if (pcb->state == ESTABLISHED)
	{
		try_reconnect = 0;
		return ERR_ALREADY;
	}
	else
	{
		xil_printf(" Rebuild tcp connection, tcp monitor \n");
		destroy_monitor(monitor);
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
		err = tcp_close(pcb);
		if (err != ERR_OK) {
			/* Free memory with abort */
			tcp_abort(pcb);
		}
	}
	return err;
}

/*-----------------------------------------------------------*/
err_t client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    if (err != ERR_OK) {
        xil_printf("Connection failed with error code: %d\n\r", err);
        return err;
    }
    xil_printf("Successfully connected to server\n\r");
    stat_tcp_con = 1;
    tcp_recv(tpcb, recv_callback);
    return ERR_OK;
}

/*-----------------------------------------------------------*/
uint8_t get_tetrada(uint8_t *data, size_t len) {
    if (data == NULL || len == 0) {
        return 0;
    }
	uint8_t lowest_byte = data[len - 1];
	xil_printf("Lowest byte %02x", lowest_byte);
    
    return (lowest_byte >> 4) & 0x0F;
}

/*-----------------------------------------------------------*/
volatile u8 flag_tcp = 0;  // static for plaicing in header?
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
		uint8_t test_tetrada = 0x0F;
		xil_printf("tetrada of command %02x", tetrada);
		switch (tetrada)
		{
		case 0x0E:
			xil_printf("TOB VALUE");
			last_cmd = CMD_TOB_VALUE_REQS;
			break;
		
		case 0x0F:
			xil_printf("WORKTYPES");
			last_cmd = CMD_WORKTYPE_REQS;
			break;

		case 0x0D:
			xil_printf("TOB POINT");
			last_cmd = CMD_TOB_POINT;
			break;

		case 0x0C:
			xil_printf("Command with C tetrada");
			if (last_byte == CMD_STOP)
			{
				xil_printf("Stop UDP send!");
				status_udp_sender = 0;
				last_cmd = CMD_STOP;
			}
			else if (last_byte == CMD_START)
			{
				xil_printf("Start UDP send!");
				status_udp_sender = 1;
				vTaskResume(xIrqTaskHandle);
				last_cmd = CMD_START;
			}
			else if (last_byte == CMD_WORKTYPE_SET)
			{
				xil_printf("Set worktype!");
				last_cmd = CMD_WORKTYPE_SET;
			}
			else if (last_byte == CMD_CTRL_AMPL)
			{
				xil_printf("Set amplifyer control mode!");
				last_cmd = CMD_CTRL_AMPL;
			}
			else if (last_byte == CMD_SEA_FILTER)
			{
				xil_printf("Set sea filter!");
				last_cmd = CMD_SEA_FILTER;
			}
			else if (last_byte == CMD_PREC_FILTER)
			{
				xil_printf("Set rain filter!");
				last_cmd = CMD_PREC_FILTER;
			}
			else if (last_byte == CMD_VELOCITY)
			{
				xil_printf("Set velocity!");
				last_cmd = CMD_VELOCITY;
			}
			else if (last_byte == CMD_FREQ_CHGE)
			{
				xil_printf("Set frequecny!");
				last_cmd = CMD_FREQ_CHGE;
			}
			else if (last_byte == CMD_TELEMETRY_REQS)
			{
				xil_printf("Get a telemetry!");
				last_cmd = CMD_TELEMETRY_REQS;
			}
			else
			{
				xil_printf("Reset faults!");
				last_cmd = CMD_RESET_FAULTS;
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
		last_cmd = CMD_BLANK_SEC;
	}
	return 0;
}

/*-----------------------------------------------------------*/
void* last_payload_from_tcp;
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

	if (p->len == 6)
	{
		xil_printf("This is extended pack!");
		extend_pack = 1;
	}

	memcpy(buf, p->payload, 6);
	ssize = p->len;
	xMessageBufferSendFromISR(xMsgBuffer, (void*)buf, ssize, NULL);
	if (tcp_sndbuf(tpcb) > p->len)
	{
		err = tcp_write(tpcb, p->payload, p->len, 1);
	}
	else{ xil_printf("no space in tcp_sndbuf\n\r"); }

	flag_tcp ^= 1;

	/* free the received pbuf */
	pbuf_free(p);

	return ERR_OK;
}
