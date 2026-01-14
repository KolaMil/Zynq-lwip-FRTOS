/*
 ============================================================================
 Name        : main.c
 Author      : FET
 Version     :
 Description : Ñommunication CPU1 with CPU0
 ============================================================================
 */

#include "main.h"

int main(void)
{
	Xil_SetTlbAttributes(SHARE_BASE, 0x14de2);

	xil_printf("\nHello bratish! I'm CPU1\r\n");
	return 0;
}
