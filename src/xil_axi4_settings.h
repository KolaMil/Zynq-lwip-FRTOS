#ifndef XIL_AXI4_SETTINGS_H
#define XIL_AXI4_SETTINGS_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"

#define DMA_CONTROL_REG_OFFSET 0x00000000
#define DMA_ADDR_LOW_OFFSET    0x00000010
#define DMA_ADDR_HIGH_OFFSET   0x00000014
#define DMA_LEN_LOW_OFFSET     0x00000018
#define DMA_LEN_HIGH_OFFSET    0x0000001C


/**************************** Type Definitions *****************************/
/**
 *
 * Write a value to a DUMMY_XIL_AXI4 register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the DUMMY_XIL_AXI4device.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void DUMMY_XIL_AXI4_mWriteReg(u32 BaseAddress, unsigned RegOffset, u32 Data)
 *
 */
#define DMA_WriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

//Remember:
//static INLINE void Xil_Out32(UINTPTR Addr, u32 Value)
//{
//#ifndef ENABLE_SAFETY
//	volatile u32 *LocalAddr = (volatile u32 *)Addr;
//	*LocalAddr = Value;
//#else
//	XStl_RegUpdate(Addr, Value);
//#endif
//}



/**
 *
 * Read a value from a DUMMY_XIL_AXI4 register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the DUMMY_XIL_AXI4 device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 * @note
 * C-style signature:
 * 	u32 DUMMY_XIL_AXI4_mReadReg(u32 BaseAddress, unsigned RegOffset)
 *
 */
#define DMA_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

//static INLINE u32 Xil_In32(UINTPTR Addr)
//{
//	return *(volatile u32 *) Addr;
//}


/************************** Function Prototypes ****************************/
/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the DUMMY_XIL_AXI4 instance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */


#endif // XIL_AXI4_SETTINGS_H
