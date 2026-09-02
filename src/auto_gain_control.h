#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "lwip_us.h"

#define ANGULAR_STEP 0x36 // 0.3
#define NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION 1200 // nominal_number_of_lines_per_revolution
#define MINIMAL_GAIN_VALUE 100

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

AUTOGAINCONTROL* create_auto_gain_control_array(uint16_t nominal_number_of_lines_per_revolution);
void filling_auto_gain_control_array(struct pbuf *p, uint16_t size_of_samples, AUTOGAINCONTROL* auto_gain_control, uint16_t line_index);
void cleaning_auto_gain_control_array(AUTOGAINCONTROL* auto_gain_control, uint16_t boundary_line_index);
void update_gain_value (uint16_t next_line_index, AUTOGAINCONTROL* auto_gain_control);
void auto_gain_control(struct pbuf *p, AUTOGAINCONTROL* auto_gain_control);
