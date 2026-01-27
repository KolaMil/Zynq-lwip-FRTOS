#ifndef SRC_TCP_STACK_H_
#define SRC_TCP_STACK_H_

#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <stdio.h>
#include <time.h>

typedef struct
{
    struct tcp_pcb *pcb;
    enum tcp_state state;
    char remote_ip_str[16];
    uint16_t remote_port;
    uint16_t flags;
    uint32_t creation_time;
    uint32_t lifetime;
    uint32_t last_rec_cmd;
    uint32_t last_update_time;
    // float throughput_tx;
    // float throughput_rx;
} tcp_monitor_t;

tcp_monitor_t* create_tcp_monitor(struct tcp_pcb *pcb);
err_t update_monitor(tcp_monitor_t *monitor);
err_t print_status(tcp_monitor_t *monitor);
#endif /* SRC_TCP_STACK_H_ */
