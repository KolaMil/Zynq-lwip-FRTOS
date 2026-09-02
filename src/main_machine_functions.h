/*
 * main_machine_functions.h
 *
 *  Created on: 31 рту. 2026 у.
 *      Author: mgre
 */

#ifndef SRC_MAIN_MACHINE_FUNCTIONS_H_
#define SRC_MAIN_MACHINE_FUNCTIONS_H_

#include "xil_types.h"
#include "xbram.h"

struct   tcp_pcb  *tcp_server_pcb_copy;
int8_t            have_got_something;


int8_t send_bytes_to_pl(void);

void bram_write32_to_fpga (XBram * p_bram, u32 b_address, u8 b_data);

u8  bram_read32_from_fpga (XBram * p_bram,  u32 b_address);

#endif /* SRC_MAIN_MACHINE_FUNCTIONS_H_ */
