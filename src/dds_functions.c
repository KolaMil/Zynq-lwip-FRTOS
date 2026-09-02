//**************************************************************************
// This file consists of functions for using of DDS IC AD9910
//
// NOTE: symbol "::" is used here as descending hierarchical relation, names are relaxed
//
// These functions are:
//
// 1 Gpio object creation
// 2 Gpio object related structures initialization
// 3 Gpio channel  0       pins direction setting
// 4 Set start of writing to DDS IC registers by PL::Control_rw_inst::state machine
// 5 Gpio channels 1, 2, 3 pins direction setting. These 3 port are clocked by MMCM fed by SYNC_CLK, not FCLK0 PS
// 6 Set start of PL::SignalGeneration_sv::state machine

// 7 Writing of 128-bit generated signal amplitude sequence.
// 8 Writing of 128-bit generated signal phase sequence.
// 9 Writing of  32-bit control word (scanning mode )
// 10 Set start of scanning. XGpio_DiscreteWrite(&Gpio2, 1, 0x000000006);
//**************************************************************************

#include "dds_functions.h"

//**********************************************************************************************************
//  Initialization of 4 AXI GPIO driver objects: 0, 1, 2, 3
//
//  Note:  xgpio.h - file with functions for AXI GPIO
//  Note:  XGpio_Initialize function contains both  XGpio_LookupConfig and XGpio_CfgInitialize inside

int axi_gpio_objects_init_0_4_5 (void)
{
    uint8_t status0, status4, status5;

    status0 = XGpio_Initialize(&Gpio0, XPAR_AXI_GPIO_0_DEVICE_ID);
    if (Status0 != XST_SUCCESS)
           {
     		return XST_FAILURE;
     	   }

    status4 = XGpio_Initialize(&Gpio4, XPAR_AXI_GPIO_4_DEVICE_ID);
        if (Status4 != XST_SUCCESS)
          {
           return XST_FAILURE;
          }
    status5 = XGpio_Initialize(&Gpio5, XPAR_AXI_GPIO_5_DEVICE_ID);
        if (Status5 != XST_SUCCESS)
          {
          return XST_FAILURE;
          }
    return 0;
}


int axi_gpio_objects_init_1_2_3 (void)
{
    int Status1, Status2, Status3;

    Status1 = XGpio_Initialize(&Gpio1, XPAR_AXI_GPIO_1_DEVICE_ID);
        if (Status1 != XST_SUCCESS)
           {
     	  	return XST_FAILURE;
     	   }
    Status3 = XGpio_Initialize(&Gpio3, XPAR_AXI_GPIO_3_DEVICE_ID);
     	if (Status3 != XST_SUCCESS)
     	   {
     	 	return XST_FAILURE;
     	   }
    Status2 = XGpio_Initialize(&Gpio2, XPAR_AXI_GPIO_2_DEVICE_ID);
        if (Status2 != XST_SUCCESS)
           {
     	   	return XST_FAILURE;
     	   }
    return 0;
}

//**********************************************************************************************************
// Set AXI GPIO_0 pins direction
//
// AXI GPIO_0 is connected to Control_rw_inst module
// AXI GPIO_0 driver has FCLK0 100 MHz signal from PS as a clock
//
// AXI GPIO_0 driver has two channels: 1 - output (2 bits), 2 - input (3 bits)
// Channel 1 is output to  ControlBus[1:0]
//                          ControlBus[0] - Active low reset
//                          ControlBus[1] - Active high w_start_fsm signal. Attention:
//                                          w_start_fsm signal from ARM must be held = 1
//                                          for the following DDS operation

// Channel 2 is input from TestBus[1:0] (in lower 2 bits)
//                         and
//                         r_o_regs_are_written signal (in upper bit)
// 2nd  argument of XGpio_SetDataDirection function is channel number. AXI GPIO_0 has 2 channels/
// Last argument of XGpio_SetDataDirection function is direction, 0 - output, 1 - input

void set_gpio_0_direction(void)
{
	XGpio_SetDataDirection(&Gpio0, 1, 0x00000000);  // outputs
	XGpio_SetDataDirection(&Gpio0, 2, 0xffffffff);  // inputs
}

//**********************************************************************************************************
// Set AXI GPIO_1, GPIO_2, GPIO_3 pins direction

void set_gpio_1_2_3_direction(void)
{
	//-------------------------------------------------------------------
    // Set AXI GPIO_1 pins direction
    //
	// AXI GPIO_1 is connected to SignalGeneration_sv_inst inside Control_rw_inst module
	// AXI GPIO_1 driver has 100 MHz signal from MMCM (PLL) clk_wiz_0 as a clock.
	//            Note:
	//            This clock is made by MMCM from delayed in LVDS drivers chain clock from
	//            DDS SYNC_OUT signal. SYNC_OUT is LVDS pair of pins. MMCM adds to it's
	//            input clock some phase shift. This shift is equal to 315 degrees at 03.06.2025
	//            and should be adjusted later to compensate actual delay in LVDS drivers chain.
	//            Note:
	//            The target of this compensation is to set MMCM driven flip-flops hold and setup
	//            time in brackets defined in AD9910 pdf. The AD9910 specification defines hold and
	//            setup requirements for profiles control signals (applied to 3 pins) related to
	//            AD9910's SYNC_OUT clock. Hold time is 0 ns, setup time is 1.8 ns.
	//            So, applied to 3 profile pins signals are driven by MMCM driven flip-flops
	//            and their delay must be adjusted.
	//
    // AXI GPIO_1 driver has two channels: 1 - output (32 bits), 2 - output (4 bits)
	// Channel 1 is output to  w_i_SG_Data[31:0]     data bus
	// Channel 2 is output to  w_i_SG_Addr [3:0]  address bus

    XGpio_SetDataDirection(&Gpio1, 1, 0x00000000);  // outputs
   	XGpio_SetDataDirection(&Gpio1, 2, 0x00000000);  // outputs

	//-------------------------------------------------------------------
    // Set AXI GPIO_2 pins direction
    //
	// AXI GPIO_2 is connected to SignalGeneration_sv_inst inside Control_rw_inst module
	// AXI GPIO_2 driver has 100 MHz signal from MMCM (PLL) clk_wiz_0 as a clock.
   	//
    // AXI GPIO_2 driver has two channels: 1 - output (3 bits), 2 - input (5 bits)
	// Channel 1 is output to:   bit 0  - w_i_SG_Write_strobe
   	//                           bit 1  - w_i_SG_StartWork
   	//                           bit 2  - w_i_gpio_ready
	// Channel 2 is input from the xlconcat_0 concatenation bus of test signals. Not used now.
   	//

    XGpio_SetDataDirection(&Gpio2, 1, 0x00000000);  // outputs
	XGpio_SetDataDirection(&Gpio2, 2, 0xffffffff);  // inputs

	//-------------------------------------------------------------------
    // Set AXI GPIO_3 pins direction
    //
	// AXI GPIO_3 is connected to SignalGeneration_sv_inst inside Control_rw_inst module
	// AXI GPIO_3 driver has 100 MHz signal from MMCM (PLL) clk_wiz_0 as a clock.
	//
    // AXI GPIO_3 driver has only 1 channel: 1 - input (4 bits)
	// Channel 1 is input from w_o_SG_write_addr_ack[3:0] bus

    XGpio_SetDataDirection(&Gpio3, 1, 0xffffffff);//����� �����������:  0 - �����, 1 - ����
}


//**********************************************************************************************************
// Start control_rw module state machine
//
// Note: Only Gpio0 with FCLK0 clock from ARM 100 MHz clock is used.
// Delay 0.1 s is included in this function

void start_control_rw_state_machine(void)
{
	//XGpio_DiscreteWrite(&Gpio0, 1, 0x000000000);
    XGpio_DiscreteWrite(&Gpio0, 1, 0x00000000);  // ControlBus[1:0] = 0.  Reset for spi initialization in Control_rw module/
     	 	                                                   // ControlBus[0] - Active low reset
     	 	                                                   // ControlBus[1] - Active high w_start_fsm signal from ARM,
     	 	                                                   // must be held = 1 for the following DDS operation
                                                               // So, Control_rw module is disabled.
    usleep(100000);                                            // Delay 0.1 s.
    //XGpio_DiscreteWrite(&Gpio0, 1, 0x000000003);
    XGpio_DiscreteWrite(&Gpio0, 1, 0x00000003);  // Control_rw::ControlBus[1:0]= 32'b0000 0000 0000 0000 0000 0000 0000 0011
     	 	                                                   // Remove reset, start fsm in Control_rw module
}


//**********************************************************************************************************
// Gpio1, Gpio2, Gpio3  are ready now. gpio_ready bit 2 is set for PL.
//
// Note: SignalGeneration_sv  r_generation_enabled <= DDS_ready & ADC_ready & StartWork;
// Channel 1 is output to: bit 0  - w_i_SG_Write_strobe
//                         bit 1  - w_i_SG_StartWork
//                         bit 2  - w_i_gpio_ready

void set_gpio_ready_bit(void)
{
	XGpio_DiscreteWrite(&Gpio2, 1, 0x00000004);
}

//**********************************************************************************************************
// Start SignalGeneration_sv_inst state_machine,
// set bit 1  - w_i_SG_StartWork
// bit2 w_i_gpio_ready must be kept equal to 1

void start_signal_generation_state_machine(void)
{
	extern u8 radio_on_air;
	radio_on_air = 1;
	XGpio_DiscreteWrite(&Gpio2, 1, 0x00000006); // Note: r_generation_enabled <= DDS_ready & ADC_ready & StartWork;
	                                             // Channel 1 is output to: bit 0  - w_i_SG_Write_strobe
	                                             //                         bit 1  - w_i_SG_StartWork
	                                             //                         bit 2  - w_i_gpio_ready;
}

//**********************************************************************************************************
// Stop SignalGeneration_sv_inst state_machine,
// set bit 1  - w_i_SG_StartWork
// bit2 w_i_gpio_ready must be kept equal to 1

void stop_signal_generation_state_machine(void)
{
	extern u8 radio_on_air;
	radio_on_air = 0;
	XGpio_DiscreteWrite(&Gpio2, 1, 0x00000004); // Note: r_generation_enabled <= DDS_ready & ADC_ready & StartWork;
	                                             // Channel 1 is output to: bit 0  - w_i_SG_Write_strobe
	                                             //                         bit 1  - w_i_SG_StartWork
	                                             //                         bit 2  - w_i_gpio_ready;
}

//**********************************************************************************************************
// Write DDS register value to SignalGeneration_sv_inst

void write_dds_register_value (
		                       u32     address,
							   u32     data
						      )
{
    int i;

	XGpio_DiscreteWrite(&Gpio1, 1, data);         // Output to BusDataSignal[31:0] (SignalGeneration_0)
	XGpio_DiscreteWrite(&Gpio1, 2, address);      // Output to BusAddrSignal[3:0]  (SignalGeneration_0)
	XGpio_DiscreteWrite(&Gpio2, 1, 0x00000005);
	i=1;
	while(i==1)
	    {
		ReadAddr = XGpio_DiscreteRead(&Gpio3, 1); // Input from SignalGeneration w_o_SG_write_addr_ack[3:0] bus
		TestAddr = ReadAddr & 0x0000000f;         // & 1111
		if(TestAddr == address) i=0;
		else                    i=1;
	    }
	XGpio_DiscreteWrite(&Gpio2, 1, 0x00000004);
}


//**********************************************************************************************************
// Write control word value to SignalGeneration_sv_inst

void write_control_word_value ( u32  data )
{
    int i;

    XGpio_DiscreteWrite(&Gpio1, 1, data);
    XGpio_DiscreteWrite(&Gpio1, 2, (u32) 0 );
    XGpio_DiscreteWrite(&Gpio2, 1, 0x00000005);
    i=1;
    while(i==1)
        {
        ReadAddr = XGpio_DiscreteRead(&Gpio3, 1);
        TestAddr = ReadAddr & 0x0000000f;
        if(TestAddr == Addr) i=0;
        else                 i=1;
        }
    XGpio_DiscreteWrite(&Gpio2, 1, 0x00000004);
}

//**********************************************************************************************************
// Set and write control word to pl SignalGeneration_sv (dds control)

void set_complex_sequence ( u8 operation_mode )
{
	u32 c_word;

	c_word = 0;

	// Set bit 0
	if (
		 (( operation_mode == 15 ) || ( operation_mode == 18 )) ||
	 	 (( operation_mode == 23 ) || ( operation_mode == 26 ))
	   )
	    {
		// 7 bit Barker
	    // Sequences of phases (DataPH#). 0 - 0 degrees, 1 - 180 degrees
	    //DataPH1 = 0x27000000;//0xe4000000;
	    DataPH1 = 0xe4000000;
	    DataPH2 = 0x00000000;
	    DataPH3 = 0x00000000;
	    DataPH4 = 0x00000000;

	    // Sequences of amplitudes (DataAM#). 0 - zero amplitude, 1 - full amplitude
	    DataAM1 = 0xfe000000;
	    DataAM2 = 0x00000000;
	    DataAM3 = 0x00000000;
	    DataAM4 = 0x00000000;
	    }

	else
	   {
	   if (
		   (( operation_mode == 16 ) || ( operation_mode == 19 )) ||
		   (( operation_mode == 24 ) || ( operation_mode == 27 )) ||
		   (( operation_mode == 31 ) || ( operation_mode == 34 ))
		  )
	       {
			// 11 bit Barker
		    // Sequences of phases (DataPH#). 0 - 0 degrees, 1 - 180 degrees
		    DataPH1 = 0xe2400000;
		    DataPH2 = 0x00000000;
		    DataPH3 = 0x00000000;
		    DataPH4 = 0x00000000;

		    // Sequences of amplitudes (DataAM#). 0 - zero amplitude, 1 - full amplitude
		    DataAM1 = 0xffe00000;
		    DataAM2 = 0x00000000;
		    DataAM3 = 0x00000000;
		    DataAM4 = 0x00000000;
	       }
	   else
	       {
		   if (
			   (( operation_mode == 17 ) || ( operation_mode == 20 )) ||
			   (( operation_mode == 25 ) || ( operation_mode == 28 )) ||
			   (( operation_mode == 32 ) || ( operation_mode == 35 )) ||
			   (( operation_mode == 40 ) || ( operation_mode == 43 )) ||
			   (( operation_mode == 46 ) || ( operation_mode == 49 )) ||
			   (( operation_mode == 52 ) || ( operation_mode == 55 ))
			  )
		       {
				// 13 bit Barker
			    // Sequences of phases (DataPH#). 0 - 0 degrees, 1 - 180 degrees
			    DataPH1 = 0xf9a80000;
			    DataPH2 = 0x00000000;
			    DataPH3 = 0x00000000;
			    DataPH4 = 0x00000000;

			    // Sequences of amplitudes (DataAM#). 0 - zero amplitude, 1 - full amplitude
			    DataAM1 = 0xfff80000;
			    DataAM2 = 0x00000000;
			    DataAM3 = 0x00000000;
			    DataAM4 = 0x00000000;
		       }
		   else
		       {
			   if (
				   (( operation_mode == 33 ) || ( operation_mode == 36 )) ||
				   (( operation_mode == 41 ) || ( operation_mode == 44 )) ||
				   (( operation_mode == 47 ) || ( operation_mode == 50 )) ||
				   (( operation_mode == 53 ) || ( operation_mode == 56 ))
				  )
			       {
					// 31 bit M-sequence
				    // Sequences of phases (DataPH#). 0 - 0 degrees, 1 - 180 degrees
				    // Sequences of phases (DataPH#). 0 - 0 degrees, 1 - 180 degrees
				    DataPH1 = 0x327DC568;
				    DataPH2 = 0x00000000;
				    DataPH3 = 0x00000000;
				    DataPH4 = 0x00000000;

				    // Sequences of amplitudes (DataAM#). 0 - zero amplitude, 1 - full amplitude
				    DataAM1 = 0xfffffffe;
				    DataAM2 = 0x00000000;
				    DataAM3 = 0x00000000;
				    DataAM4 = 0x00000000;
			       }
			   else
			       {

				   if (
					   (( operation_mode == 42 ) || ( operation_mode == 45 )) ||
					   (( operation_mode == 48 ) || ( operation_mode == 51 )) ||
					   (( operation_mode == 54 ) || ( operation_mode == 57 ))
					  )
				       {
						// 63 bit M-sequence
					    // Sequences of phases (DataPH#). 0 - 0 degrees, 1 - 180 degrees
					    // Sequences of phases (DataPH#). 0 - 0 degrees, 1 - 180 degrees
					    DataPH1 = 0xA7E83848;
					    DataPH2 = 0xD96BBCC4;
					    DataPH3 = 0x00000000;
					    DataPH4 = 0x00000000;

					    // Sequences of amplitudes (DataAM#). 0 - zero amplitude, 1 - full amplitude
					    DataAM1 = 0xffffffff;
					    DataAM2 = 0xfffffffe;
					    DataAM3 = 0x00000000;
					    DataAM4 = 0x00000000;

				       }
				   else
				       {
					   if (
						   (( operation_mode == 58 ) || ( operation_mode == 59 ))
						  )
					       {
							// 127  bit M-sequence
						    // Sequences of phases (DataPH#). 0 - 0 degrees, 1 - 180 degrees
						    // Sequences of phases (DataPH#). 0 - 0 degrees, 1 - 180 degrees
						    DataPH1 = 0xA81BE0B0;
						    DataPH2 = 0x87462729;
						    DataPH3 = 0xA5EBB8F3;
						    DataPH4 = 0x2455B3FA;


						    // Sequences of amplitudes (DataAM#). 0 - zero amplitude, 1 - full amplitude
						    DataAM1 = 0xffffffff;
						    DataAM2 = 0xffffffff;
						    DataAM3 = 0xffffffff;
						    DataAM4 = 0xfffffffe;
					       }
				       }
			       }
		       }
	       }
	   }

    write_dds_register_value((u32)1, DataPH1 );
    write_dds_register_value((u32)2, DataPH2 );
    write_dds_register_value((u32)3, DataPH3 );
    write_dds_register_value((u32)4, DataPH4 );
    write_dds_register_value((u32)5, DataAM1 );
    write_dds_register_value((u32)6, DataAM2 );
    write_dds_register_value((u32)7, DataAM3 );
    write_dds_register_value((u32)8, DataAM4 );
}
