#include "axi4lt_connect.h"

void DMA_start(u32 *buffer)
{
    DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR,
	    		 (UINTPTR)0x00000010,
				 (u32) (&buffer)
                );

	DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR,
			     (UINTPTR)0x00000014,
				 (u32)0x00000000
                );

	DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR,
			     (UINTPTR)0x00000018,
				 (u32)8192
                );

	DMA_WriteReg((UINTPTR)XPAR_S2M_0_BASEADDR,
			     (UINTPTR)0x0000001C,
				 (u32)0x00000000
                );
}