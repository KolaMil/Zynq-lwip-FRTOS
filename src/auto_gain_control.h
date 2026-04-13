//
// Created by nmil on 13.04.2026.
//

#ifndef XILINX_MCU_AUTO_GAIN_CONTROL_H
#define XILINX_MCU_AUTO_GAIN_CONTROL_H
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t counter_repetition;
    bool flag_cgange;
    uint8_t curent_gain;
} AUTOGAINCONTROL;

typedef struct
{
    uint16_t max_permissible_value;
    uint8_t weakening_zone;
    uint8_t number_of_permissible_revolutions;
} AUTOGAINCONTROLCONSTANTS;

uint8_t angle_change_step;

AUTOGAINCONTROLCONSTANTS autogain_control_constants[20] = {
    {0xEC54, 5, 3}, // 0,25 50
    {0xEC54, 5, 3}, // 0,25 100
    {0xEC54, 4, 3}, // 0,5  50
    {0xEC54, 4, 3}, // 0,5  100
    {0xEC54, 4, 3}, // 0,75 50
    {0xEC54, 4, 3}, // 0,75 100
    {0xEC54, 3, 3}, // 1,5  50
    {0xEC54, 3, 3}, // 1,5  100
    {0xEC54, 3, 3}, // 3    50
    {0xEC54, 3, 3}, // 3    100
    {0xEC54, 3, 3}, // 6    50
    {0xEC54, 3, 3}, // 6    100
    {0xEC54, 2, 2}, // 12   50
    {0xEC54, 2, 2}, // 12   100
    {0xEC54, 2, 2}, // 12   150
    {0xEC54, 2, 2}, // 24   100
    {0xEC54, 2, 2}, // 24   150
    {0xEC54, 2, 2}, // 24   200
    {0xEC54, 1, 1}, // 48   150
    {0xEC54, 1, 1}  // 48   200
};

#endif //XILINX_MCU_AUTO_GAIN_CONTROL_H