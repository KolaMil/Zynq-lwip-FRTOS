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

    IP4_ADDR(&ipaddr, 192,168,1,10);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&gw, 192,168,1,1);

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
void udp_connection(uint8_t *data, size_t size)
{
	udp_pcb_conn = udp_new();
	if(!udp_pcb_conn)
	{
		xil_printf("Cannot create UDP PCB\r\n");
		vTaskDelete(NULL);
		return;
	}
	ip_addr_t remote_ip;
	IP4_ADDR(&remote_ip, 192, 168, 1, 100);
	sender = add_sender(udp_pcb_conn, data, size);
	if (sender == NULL)
	{
		vTaskDelete(NULL);
		return;
	}
	udp_connect(udp_pcb_conn, &remote_ip, 5005);
}

/*-----------------------------------------------------------*/
void udp_package_send(void) // Maybe err_t type
{
	err_t err = send_udp_data(sender);
}

/*-----------------------------------------------------------*/
volatile u8 stat_tcp_con = 0;
void tcp_connection_cl(void)
{
	err_t err;
	ip_addr_t remote_addr;
#if LWIP_IPV6==1
	remote_addr.type= IPADDR_TYPE_V6;
	err = inet6_aton(TCP_SERVER_IPV6_ADDRESS, &remote_addr);
#else
	err = inet_aton(TCP_SERVER_IP_ADDRESS, &remote_addr);
#endif /* LWIP_IPV6 */
	/* create new TCP PCB structure */
	tcp_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!tcp_pcb)
	{
		xil_printf("Error creating PCB. Out of Memory\n\r");
		return;
	}
	err = tcp_connect(tcp_pcb, &remote_addr, TCP_PORT, client_connected);
	if (err)
	{
		xil_printf("Error on tcp_connect: %d\r\n", err);
		tcp_client_close(tcp_pcb);
		return;
	}
}

/*-----------------------------------------------------------*/
uint8_t state_tcp_connection(void)
{
//	tcp_pcb->state
}

/*-----------------------------------------------------------*/
void tcp_client_close(struct tcp_pcb *pcb)
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
volatile u8 flag_tcp = 0;  // static for plaicing in header?
volatile u8 status_udp_sender = 0;
uint8_t cmd_array[14] = {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xF0, 0xD0, 0xE0};
int parse_msg(void* p, size_t size)
{
	uint8_t val = *((uint8_t*) p);
	if (extend_pack)
	{
		xil_printf("Make a blind sector! \n");
		// massive with data send to PL-part in BRAM
		// coming soon
		last_cmd = CMD_BLANK_SEC;
	}

	// uint8_t value = (uint8_t)p[2];

	switch (val)
	{
	case CMD_STOP:
		xil_printf("Stop working");
		default_state[3] += 1;
		// destroy_sender(sender);
		status_udp_sender = 0;
		break;

	case CMD_START:
		xil_printf("Start working\n");
		default_state[3] += 2;
		vTaskResume(xIrqTaskHandle);
		status_udp_sender = 1;
		last_cmd = CMD_START;
		break;

	case CMD_WORK_TYPE:
		xil_printf("Requestto change work type\n");
		
		last_cmd = CMD_WORK_TYPE;
		break;

	case CMD_CTRL_AMPL:
		xil_printf("Request ampliefyer\n");
		break;

	case CMD_SEA_FILTER:
		xil_printf("Request sea filter\n");
		break;

	case CMD_PREC_FILTER:
		xil_printf("Request rain filter\n");
		break;

	case CMD_VELOCITY:
		xil_printf("Request velocity\n");
		
		break;

	case CMD_FREQ_CHGE:
		xil_printf("Change frequency\n");
		break;

	case CMD_TELEMETRY_REQS:
		xil_printf("Request telemetry\n");
		break;

	case CMD_RESET_FAULTS:
		xil_printf("Resetting faults\n");
		break;

	case CMD_WORKTYPE_REQS:
		xil_printf("Request worktype\n");
		break;

	case CMD_TOB_POINT:
		xil_printf("Request tob point\n");
		break;

	case CMD_TOB_VALUE_REQS:
		xil_printf("Request value of TOB\n");
		break;

	default:
		xil_printf("Unknown cmd");
		break;
	}
	return 0;
}

/*-----------------------------------------------------------*/
void* last_payload_from_tcp;
err_t recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	if (!p)
	{
		tcp_close(tpcb);
		tcp_recv(tpcb, NULL);
		return ERR_OK;
	}
	extend_pack = 0;
	/* pass information about the message to stack */
	tcp_recved(tpcb, p->len);
	uint8_t buf[6] = {0};
	if (p->len == 6)
	{
		xil_printf("This is extended pack!");  // for debug
		extend_pack = 1;
	}
	
	memcpy(buf, p->payload, 6);
	xQueueSendFromISR(xTcpMsgQueue, buf, NULL);
	if (tcp_sndbuf(tpcb) > p->len)
	{
		err = tcp_write(tpcb, p->payload, p->len, 1);
	}
	else
	{
		xil_printf("no space in tcp_sndbuf\n\r");
	}

	flag_tcp ^= 1;

	/* free the received pbuf */
	pbuf_free(p);

	return ERR_OK;
}
