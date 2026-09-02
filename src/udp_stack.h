#ifndef SRC_UDP_STACK_H_
#define SRC_UDP_STACK_H_

#include "lwip/udp.h"

typedef struct
{
    struct pbuf *udp_pbuf;
    struct udp_pcb *pcb;
    uint32_t err_count;
}udp_sender_t;

uint8_t count;

udp_sender_t* add_sender(struct udp_pcb *pcb, uint8_t* data, size_t len);
err_t send_udp_data(udp_sender_t *sender);
void destroy_sender(udp_sender_t *sender);

#endif /* SRC_UDP_STACK_H_ */
