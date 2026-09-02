#include "main_machine_functions.h"
#include "xil_cache.h"

int8_t send_bytes_to_pl(void)
    {
	return 0;
    }

// Writes 4 bytes. b_address is relative address, measured in 32 bit words.
// b_address is counting from MemBaseAddress of called bram
void bram_write32_to_fpga (XBram * p_bram, u32 b_address, u8 b_data)
    {
	Xil_Out32 ( ( (p_bram->Config.MemBaseAddress) + (4 * b_address) ) & (UINTPTR)0xfffffffffffffffc,
			  (0x00000000 | b_data)
			  );
	//Xil_DCacheFlushRange ( (INTPTR) (
	//		            		      ( (p_bram->Config.MemBaseAddress) + (4 * b_address) ) & (UINTPTR)0xfffffffffffffffc
	//							    ),
	//		             4
	//		             );

    }

// Reads 4 bytes. b_address is relative address, measured in 32 bit words.
// b_address is counting from MemBaseAddress of called bram
u8 bram_read32_from_fpga (XBram * p_bram,  u32 b_address)
    {
    return Xil_In32 ( ( (p_bram->Config.MemBaseAddress) + (4 * b_address) ) & (UINTPTR)0xfffffffffffffffc ) & 0x000000ff;
    }
