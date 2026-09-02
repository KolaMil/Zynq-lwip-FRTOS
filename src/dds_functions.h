#pragma once

#include <stdio.h>
#include "xil_printf.h"
#include "xil_io.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xgpio.h"
#include "xgpiops.h"
#include "sleep.h"

uint32_t DataDDS1, DataDDS2, DDSAddr;
uint32_t DataAM1, DataAM2, DataAM3, DataAM4;
uint32_t DataPH1, DataPH2, DataPH3, DataPH4;
uint32_t control_word;
uint32_t Addr, TestAddr, ReadAddr;
XGpio Gpio0; /* The Instance of the AXI GPIO Driver */
XGpio Gpio1; /* The Instance of the AXI GPIO Driver */
XGpio Gpio2; /* The Instance of the AXI GPIO Driver */
XGpio Gpio3; /* The Instance of the AXI GPIO Driver */
XGpio Gpio4;
XGpio Gpio5;

//**********************************************************************************************************
//  Initialization of  AXI GPIO driver objects
//
//  Note:  xgpio.h - file with functions for AXI GPIO
//  Note:  XGpio_Initialize function contains both  XGpio_LookupConfig and XGpio_CfgInitialize inside

int axi_gpio_objects_init_1_2_3 (void);
int axi_gpio_objects_init_0_4_5 (void);

//**********************************************************************************************************
// Set AXI GPIO_0 pins direction
//
// AXI GPIO_0 is connected to Control_rw_inst module
// AXI GPIO_0 driver has FCLK0 100 MHz signal from PS as a clock
//
// AXI GPIO_0 driver has two channels: 1 - output (2 bits), 2 - input (3 bits)
// Channel 1 is output to  ControlBus[1:0]
// Channel 2 is input from TestBus[1:0] (in lower 2 bits)
//                         and
//                         r_o_regs_are_written signal (in upper bit)
// 2nd  argument of XGpio_SetDataDirection function is channel number. AXI GPIO_0 has 2 channels/
// Last argument of XGpio_SetDataDirection function is direction, 0 - output, 1 - input

void set_gpio_0_direction(void);

//**********************************************************************************************************
// Set AXI GPIO_1, GPIO_2, GPIO_3 pins direction
//
// AXI GPIO_1, GPIO_2, GPIO_3 are connected to SignalGeneration_sv_inst inside Control_rw_inst module
// AXI GPIO_1, GPIO_2, GPIO_3 has 100 MHz signal from MMCM (PLL) clk_wiz_0 as a clock.
//
// AXI GPIO_1 driver has two channels: 1 - output (32 bits), 2 - output (4 bits)
// Channel 1 is output to:   w_i_SG_Data[31:0]     data bus
// Channel 2 is output to:   w_i_SG_Addr [3:0]  address bus
//
// AXI GPIO_2 driver has two channels: 1 - output (3 bits), 2 - input (5 bits)
// Channel 1 is output to:   bit 0  - w_i_SG_Write_strobe
//                           bit 1  - w_i_SG_StartWork
//                           bit 2  - w_i_gpio_ready
// Channel 2 - not used now.
//
// AXI GPIO_3 driver has only 1 channel: 1 - input (4 bits)
// Channel 1 is input from   w_o_SG_write_addr_ack[3:0] bus

void set_gpio_1_2_3_direction(void);

//**********************************************************************************************************
// Start control_rw module state machine
// Note: Only Gpio0 with FCLK0 clock from ARM 100 MHz clock is used.
// Delay 0.1 s is included in tjis function

void start_control_rw_state_machine(void);

//**********************************************************************************************************
// Gpio1, Gpio2, Gpio3  are ready now. gpio_ready bit 2 is set for PL.

void set_gpio_ready_bit(void);

//**********************************************************************************************************
// Start SignalGeneration_sv state_machine

void start_signal_generation_state_machine(void);

//**********************************************************************************************************
// Stop SignalGeneration_sv_inst state_machine,

void stop_signal_generation_state_machine(void);

//**********************************************************************************************************
// Write DDS register

void write_dds_register_value (u32 address, u32 data);

//**********************************************************************************************************
// Write control word value to SignalGeneration_sv_inst

void write_control_word_value ( u32  data );

//**********************************************************************************************************
// Set and write control word to pl SignalGeneration_sv (dds control)
//
void set_complex_sequence ( u8 operation_mode );
