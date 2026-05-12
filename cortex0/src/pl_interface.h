#include "xil_printf.h"
#include "xil_io.h"
// #include "xbram.h"

#ifdef ENABLED_BRAM
#define SRC_HHH_H_

#define DMA_WriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

#ifndef SDT
#define BRAM_DEVICE_ID		XPAR_BRAM_0_DEVICE_ID
#else
#define BRAM_DEVICE_ID		XPAR_XBRAM_0_BASEADDR
#endif

XBram Bram;

#ifndef SDT
int BramExample(u16 DeviceId);
#else
int BramExample(UINTPTR BaseAddress);
#endif

static void InitializeECC(XBram_Config *ConfigPtr, u32 EffectiveAddr);
void send_to_PL (uint8_t *message, uint16_t size);

#endif
