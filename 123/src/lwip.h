#ifndef __LWIP_H_
#define __LWIP_H_

/*-----------------------------------------------------------*/
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "message_buffer.h"

#include "lwip/tcp.h"
#include "lwip/inet.h"
#include "lwip/udp.h"
#include "lwip/init.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "lwip/pbuf.h"
#include "tcp_stack.h"
#include "udp_stack.h"
#include "cmd.h"

/*-----------------------------------------------------------*/
#define LOCAL_IP_ADDRESS    "192.168.1.10"
#define REMOTE_IP_ADDRESS   "192.168.1.100"
#define LOCAL_NET_MASK      "255.255.255.0"
#define LOCAL_GATEWAY       "192.168.1.1"
#define TCP_REMOTE_PORT     ((uint16_t)7)
#define UDP_REMOTE_PORT     ((uint16_t)5005)

#define TCP_PORT 7

/*-----------------------------------------------------------*/
struct udp_pcb *udp_pcb_conn;
struct tcp_pcb *tcp_pcb;
struct netif server_netif;
udp_sender_t *sender;
tcp_monitor_t *monitor;
extern volatile QueueHandle_t xTcpMsgQueue;
extern volatile TaskHandle_t xIrqTaskHandle;
extern volatile MessageBufferHandle_t xMsgBuffer;

uint16_t last_cmd;

extern uint8_t default_state[8192];
extern uint8_t extend_pack;
extern uint8_t try_reconnect;
/*-----------------------------------------------------------*/
void lwip_network_setup(void);
err_t udp_connection(uint8_t *data, size_t size);
err_t tcp_connection_cl(void);
err_t udp_package_send(void);
err_t tcp_client_close(struct tcp_pcb *pcb);
err_t client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
int parse_msg(void* p, size_t size);
err_t need_reconnect(struct tcp_pcb *pcb);
err_t recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

#endif
