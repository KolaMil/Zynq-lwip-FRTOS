#include "tcp_stack.h"

const char* tcp_state_to_string(enum tcp_state state) {
    switch(state) {
        case CLOSED:      return "CLOSED";
        case LISTEN:      return "LISTEN";
        case SYN_SENT:    return "SYN_SENT";
        case SYN_RCVD:    return "SYN_RCVD";
        case ESTABLISHED: return "ESTABLISHED";
        case FIN_WAIT_1:  return "FIN_WAIT_1";
        case FIN_WAIT_2:  return "FIN_WAIT_2";
        case CLOSE_WAIT:  return "CLOSE_WAIT";
        case CLOSING:     return "CLOSING";
        case LAST_ACK:    return "LAST_ACK";
        case TIME_WAIT:   return "TIME_WAIT";
        default:          return "UNKNOWN";
    }
}

tcp_monitor_t* create_tcp_monitor(struct tcp_pcb *pcb) {
    tcp_monitor_t *monitor;

    monitor = (tcp_monitor_t*)mem_malloc(sizeof(tcp_monitor_t));
    if (monitor == NULL) {
        return NULL;
    }
    
    memset(monitor, 0, sizeof(tcp_monitor_t));
    
    monitor->pcb = pcb;
    monitor->creation_time = xTaskGetTickCount();
    monitor->last_update_time = xTaskGetTickCount();
    monitor->remote_port = pcb->remote_port;
    monitor->state = pcb->state;
    monitor->flags = pcb->flags;
    if (!ip_addr_isany(&pcb->remote_ip)) {
            ipaddr_ntoa_r(&pcb->remote_ip, monitor->remote_ip_str, sizeof(monitor->remote_ip_str));
        } else {
            strcpy(monitor->remote_ip_str, "0.0.0.0");
        }
    
    return monitor;
}

err_t update_monitor(tcp_monitor_t *monitor){
    
    if (monitor == NULL || monitor->pcb == NULL) {
        return ERR_MEM;
    }
    struct tcp_pcb *pcb = monitor->pcb;
    monitor->state = pcb->state;
    monitor->flags = pcb->flags;
    monitor->remote_port = pcb->remote_port;
    monitor->last_update_time = xTaskGetTickCount();
    monitor->lifetime = monitor->last_update_time - monitor->creation_time;
    ipaddr_ntoa_r(&pcb->remote_ip, monitor->remote_ip_str, sizeof(monitor->remote_ip_str));

    return ERR_OK;
}

err_t print_status(tcp_monitor_t *monitor){
    if (monitor == NULL || monitor->pcb == NULL) {
        return ERR_MEM;
    }
    xil_printf("\n======== TCP Connection Monitor ========\n");
    xil_printf(" Remote: %s:%d\n", monitor->remote_ip_str, monitor->remote_port);
    xil_printf(" State:  %s (%d)\n", tcp_state_to_string(monitor->state), monitor->state);
    xil_printf(" Lifetime: %d\n", monitor->lifetime);

    return ERR_OK;
}
