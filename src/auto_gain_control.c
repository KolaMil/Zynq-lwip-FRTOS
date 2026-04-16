//
// Created by nmil on 13.04.2026.
//
#include "auto_gain_control.h"

#include "main.h"

AUTOGAINCONTROL* create_auto_gain_control_array(uint16_t nominal_number_of_lines_per_revolution)
{
    AUTOGAINCONTROL *auto_gain_control = malloc(sizeof(AUTOGAINCONTROL) * nominal_number_of_lines_per_revolution);
    if (auto_gain_control == NULL) return NULL;

    return auto_gain_control;
}

uint8_t error = 0x05;
void filling_auto_gain_control_array(uint16_t* samples, uint16_t size_of_samples, AUTOGAINCONTROL* auto_gain_control, uint16_t line_index)
{
    for (uint16_t sample_index; sample_index < size_of_samples; sample_index++)
    {
        if (samples[sample_index] > autogain_control_constants[0].max_permissible_value)
        {
            for (uint8_t weakening_line_index = line_index - autogain_control_constants[0].weakening_zone * 2; weakening_line_index <= line_index + autogain_control_constants[0].weakening_zone * 2; weakening_line_index++)
            {
                auto_gain_control[weakening_line_index].counter_repetition++;
                auto_gain_control[weakening_line_index].flag_cgange = 1;
            }
            continue;
        }
    }
    if (auto_gain_control->counter_repetition > autogain_control_constants[0].number_of_permissible_revolutions)
    {
        auto_gain_control->curent_gain -= 10;
    }
    cleaning_auto_gain_control_array(auto_gain_control, line_index - (autogain_control_constants[0].weakening_zone + 1));
}

void cleaning_auto_gain_control_array(AUTOGAINCONTROL* auto_gain_control, uint16_t boundary_line_index)
{
    if (!auto_gain_control->flag_cgange)
    {
        auto_gain_control->counter_repetition = 0;
        auto_gain_control->curent_gain = 0x80;
    }
}

void auto_gain_control(uint16_t start_azimuth, uint16_t end_azimuth, uint16_t* samples, uint16_t size_of_samples, AUTOGAINCONTROL* auto_gain_control)
{
    uint16_t line_index = start_azimuth / 0x36;
    filling_auto_gain_control_array(samples, 62, auto_gain_control, line_index);
}