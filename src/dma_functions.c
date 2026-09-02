#include "xparameters.h"
#include "xil_axi4_settings.h"
#include "dma_functions.h"
#include "xil_io.h"

void DMA_Write_Destination_Address(UINTPTR Destination_Address)
{

DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR,
	         (UINTPTR)DMA_ADDR_LOW_OFFSET,
		     (u32)Destination_Address
		     );

DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR,
	         (UINTPTR)DMA_ADDR_HIGH_OFFSET,
		     (u32)0xffffffff//0x00000000
		     );

}
