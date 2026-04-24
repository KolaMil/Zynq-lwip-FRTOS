#include "pl_interface.h"

void send_to_PL (void *messege, uint16_t size)
{
    uint8_t *mesg = (uint8_t *)messege;
    xil_printf(" mesg size: %u\r\n", size);
    for (uint16_t i = 0; i < size; i++)
    {
        xil_printf("%u:  %02x\n", i, mesg[i]);
    }
}
