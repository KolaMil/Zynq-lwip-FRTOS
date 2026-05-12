#include "pl_interface.h"

// #ifndef SDT
// int BramExample(u16 DeviceId)
// #else
// int BramExample(UINTPTR BaseAddress)
// #endif
// {
// 	int Status;
// 	XBram_Config *ConfigPtr;

// 	/*
// 	 * Initialize the BRAM driver. If an error occurs then exit
// 	 */

// 	/*
// 	 * Lookup configuration data in the device configuration table.
// 	 * Use this configuration info down below when initializing this
// 	 * driver.
// 	 */
// #ifndef SDT
// 	ConfigPtr = XBram_LookupConfig(DeviceId);
// #else
// 	ConfigPtr = XBram_LookupConfig(BaseAddress);
// #endif
// 	if (ConfigPtr == (XBram_Config *) NULL) {
// 		return XST_FAILURE;
// 	}

// 	Status = XBram_CfgInitialize(&Bram, ConfigPtr,
// 				     ConfigPtr->CtrlBaseAddress);
// 	if (Status != XST_SUCCESS) {
// 		return XST_FAILURE;
// 	}


//         InitializeECC(ConfigPtr, ConfigPtr->CtrlBaseAddress);


// 	/*
// 	 * Execute the BRAM driver selftest.
// 	 */
// 	Status = XBram_SelfTest(&Bram, 0);
// 	if (Status != XST_SUCCESS) {
// 		return XST_FAILURE;
// 	}

// 	return XST_SUCCESS;
// }

// void InitializeECC(XBram_Config *ConfigPtr, u32 EffectiveAddr)
// {
// 	u32 Addr;
// 	volatile u32 Data;

// 	if (ConfigPtr->EccPresent &&
// 	    ConfigPtr->EccOnOffRegister &&
// 	    ConfigPtr->EccOnOffResetValue == 0 &&
// 	    ConfigPtr->WriteAccess != 0) {
// 		for (Addr = ConfigPtr->MemBaseAddress;
// 		     Addr < ConfigPtr->MemHighAddress; Addr+=4) {
// 			Data = XBram_In32(Addr);
// 			XBram_Out32(Addr, Data);
// 		}
// 		XBram_WriteReg(EffectiveAddr, XBRAM_ECC_ON_OFF_OFFSET, 1);
// 	}
// }

void send_to_PL (uint8_t *message, uint16_t size)
{
    uint8_t *msg = message;
    xil_printf(" Message size: %u\r\n", size);
    for (uint16_t i = 0; i < size; i++)
    {
        xil_printf("%u:  %02x\n", i, msg[i]);
    }
}
