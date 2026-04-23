/*
 ============================================================================
 Name        : cmd.h
 Author      :
 Version     :
 Description : HEX commands TCP protocol
 ============================================================================
 */

#ifndef SRC_CMD_H_
#define SRC_CMD_H_

#pragma once

#define CMD_STOP				0xC0
#define CMD_START           	0xC1
#define CMD_WORKTYPE_SET		0xC2        // 05С2
#define CMD_CTRL_AMPL			0xC3        // 00C3-FFC3
#define CMD_CTRL_AMPL_AUTO_VALUE 0xFF
#define CMD_CTRL_AMPL_MAX_VALUE	0x7F
#define CMD_CTRL_AMPL_MIN_VALUE	0x00
#define CMD_SEA_FILTER			0xC4        // 00C4-FFC4
#define CMD_PREC_FILTER			0xC5        // 00C5-FFC5
#define CMD_VELOCITY			0xC6        // 01C6-02C6-?
#define CMD_BLANK_SEC			0xC7        // 00 0A 00 14 01 C7
#define CMD_FREQ_CHGE			0xC8        // 81 C8       8(0-9) frquency, 1(1-4) number of sintezator
#define CMD_TELEMETRY_REQS		0x00C9      // 00C9 telemetry
#define CMD_RESET_FAULTS		0x01C9  	// 01C9 reset faults  //

#define CMD_WORKTYPE_REQS		0x00F   	// 00F2-00F8
#define CMD_TOB_POINT			0xD	        // 08 98 D0 it is value = 2200dec
#define CMD_TOB_VALUE_REQS		0xE

static uint16_t cmd_pull[14] = {CMD_STOP, CMD_START, CMD_WORKTYPE_SET, CMD_CTRL_AMPL, CMD_SEA_FILTER, CMD_PREC_FILTER, CMD_VELOCITY, CMD_BLANK_SEC,
 CMD_FREQ_CHGE, CMD_TELEMETRY_REQS, CMD_RESET_FAULTS, CMD_WORKTYPE_REQS, CMD_TOB_POINT, CMD_TOB_VALUE_REQS};

/* либо структура состоящая из названия команды | ссылки на ответствунную задачу | флаг актульности данных */

#endif /* SRC_CMD_H_ */
