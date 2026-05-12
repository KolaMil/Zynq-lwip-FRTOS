#include "xil_types.h"
#include "xil_io.h"
#include "xstatus.h"
#include "xparameters.h"

#define DMA_CONTROL_REG_OFFSET 0x00000000
#define DMA_ADDR_LOW_OFFSET    0x00000010
#define DMA_ADDR_HIGH_OFFSET   0x00000014
#define DMA_LEN_LOW_OFFSET     0x00000018
#define DMA_LEN_HIGH_OFFSET    0x0000001C

#define DMA_WriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

#define DMA_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

volatile u16 dma_buffer[4096] __attribute__ ((aligned (32)));

// void DMA_start(u32 *buffer);