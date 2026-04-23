#ifndef __CPU1_INIT_H_
#define __CPU1_INIT_H_

/*-----------------------------------------------------------*/
#include "xil_io.h"

/*-----------------------------------------------------------*/
#define INTC_DEVICE_ID	     XPAR_SCUGIC_SINGLE_DEVICE_ID
#define SHARE_BASE           0xffff0000
#define CPU1_START_ADDR      0xfffffff0
#define CPU1_START_MEM       0x10000000

/*-----------------------------------------------------------*/
#define SOFT_INTR_ID_TO_CPU0 0
#define SOFT_INTR_ID_TO_CPU1 1
#define CPU1_ID              2

/*-----------------------------------------------------------*/
#define sev()                __asm__("sev")

/*-----------------------------------------------------------*/
void start_cpu1();

#endif
