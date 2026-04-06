/*
 ============================================================================
 Name        : udp_stack.c
 Author      :
 Version     :
 Description : UDP support structure
 ============================================================================
 */

#include "udp_stack.h"
#include "string.h"
#include "xil_printf.h"

udp_sender_t* add_sender(struct udp_pcb *pcb, uint8_t* data, size_t len) {
    udp_sender_t *sender = mem_malloc(sizeof(udp_sender_t));
    memset(sender, 0, sizeof(udp_sender_t));

    if (!sender || !pcb) return NULL;

    sender->udp_pbuf = pbuf_alloc_reference(data, len, PBUF_POOL);
    if (!sender->udp_pbuf) {
        xil_printf("Failememd to alloc pbuf\r\n");
        mem_free(sender);
        return NULL;
    }
    sender->pcb = pcb;
    xil_printf("UDP sender initialized!\r\n");
    
    return sender;
}

err_t send_udp_data(udp_sender_t *sender)
{
    if (!sender || !sender->udp_pbuf) return ERR_VAL;
    
    err_t err = udp_send(sender->pcb, sender->udp_pbuf);
    count += 1;
    if (err != ERR_OK) 
    {
        sender->err_count++;
        xil_printf("Error sending UDP packet\r\n");
        if (sender->err_count % 1000 == 0) {
            xil_printf("UDP send errors: %lu\r\n", sender->err_count); 
        }
        return err;
    }

    ((u8 *)sender->udp_pbuf->payload)[2] = ((u8 *)sender->udp_pbuf->payload)[2] + 1;
    if (count == 10)
    {
        ((u8 *)sender->udp_pbuf->payload)[2] = 0;
    }
    
    return err;
}

void destroy_sender(udp_sender_t *sender)
{
    if (sender) {
        if (sender->udp_pbuf && sender->udp_pbuf->ref == 0) {
            pbuf_free(sender->udp_pbuf);
        }
        mem_free(sender);
    }
}
