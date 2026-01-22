#ifndef SRC_CMD_H_
#define SRC_CMD_H_

#pragma once

#define CMD_STOP				0xC0
#define CMD_START           	0xC1
#define CMD_WORK_TYPE			0xC2
#define CMD_CTRL_AMPL			0xC3
#define CMD_SEA_FILTER			0xC4
#define CMD_PREC_FILTER			0xC5
#define CMD_VELOCITY			0xC6
#define CMD_BLANK_SEC			0xC7
#define CMD_FREQ_CHGE			0xC8
#define CMD_TELEMETRY_REQS		0x00C9      // 00C9 telemetry
#define CMD_RESET_FAULTS		0x01C9  	// 01C9 reset faults

#define CMD_WORKTYPE_REQS		0x00F   	// 00F2-00F8
#define CMD_TOB_POINT			0xD		// 08 98 D0 it is value = 2200dec
#define CMD_TOB_VALUE_REQS		0xE

#endif /* SRC_CMD_H_ */
