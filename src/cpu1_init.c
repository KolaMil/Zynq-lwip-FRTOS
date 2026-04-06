/*
 ============================================================================
 Name        : cpu1_init.c
 Author      :
 Version     :
 Description : Start CPU1 with CPU0
 ============================================================================
 */

#include "cpu1_init.h"

/*-----------------------------------------------------------*/
void start_cpu1()
{
	Xil_Out32(CPU1_START_ADDR, CPU1_START_MEM);
	dmb();
	sev();
}
