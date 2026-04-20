//
// Created by nmil on 13.04.2026.
//
#include "auto_gain_control.h"

#include "xil_printf.h"
static 	uint8_t old_gain_value = 127;

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

AUTOGAINCONTROL* create_auto_gain_control_array(uint16_t nominal_number_of_lines_per_revolution)
{
    AUTOGAINCONTROL *auto_gain_control = malloc(sizeof(AUTOGAINCONTROL) * nominal_number_of_lines_per_revolution);
    for (uint16_t i = 0; i < nominal_number_of_lines_per_revolution; i++)
    {
        auto_gain_control[i].counter_repetition = 0;
        auto_gain_control[i].flag_cgange = 0;
        auto_gain_control[i].curent_gain = 127;
    }
    if (auto_gain_control == NULL) return NULL;
    return auto_gain_control;
}

uint8_t error = 0x05;
void filling_auto_gain_control_array(uint16_t* samples, uint16_t size_of_samples, AUTOGAINCONTROL* auto_gain_control, uint16_t line_index)
{
    for (uint16_t sample_index = 0; sample_index < size_of_samples; sample_index++)
    {
        if (samples[sample_index] > autogain_control_constants[0].max_permissible_value)
        {
            for (int16_t weakening_line_index = autogain_control_constants[0].weakening_zone * (-1); weakening_line_index <= autogain_control_constants[0].weakening_zone; weakening_line_index++)
            {
                int16_t real_weakening_line_index = ((line_index + NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION) + weakening_line_index) % NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION;
                if (!auto_gain_control[real_weakening_line_index].flag_cgange)
                {
                    auto_gain_control[real_weakening_line_index].counter_repetition++;
                    if (auto_gain_control[real_weakening_line_index].counter_repetition > autogain_control_constants[0].number_of_permissible_revolutions)
                    {
                        if (auto_gain_control[real_weakening_line_index].curent_gain - 10 <= MINIMAL_SAMPLES_VALUE)
                        {
                            auto_gain_control[real_weakening_line_index].curent_gain = MINIMAL_SAMPLES_VALUE;
                        }
                        else
                        {
                            auto_gain_control[real_weakening_line_index].curent_gain -= 10;
                        }
                    }
                    auto_gain_control[real_weakening_line_index].flag_cgange = 1;
                }
            }
            break;
        }
    }
    cleaning_auto_gain_control_array(auto_gain_control, ((line_index + NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION) - (autogain_control_constants[0].weakening_zone + 1)) % NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION);
    update_gain_value((line_index + 1) % NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION, auto_gain_control);
}

void cleaning_auto_gain_control_array(AUTOGAINCONTROL* auto_gain_control, uint16_t boundary_line_index)
{
    if (!auto_gain_control[boundary_line_index].flag_cgange)
    {
        auto_gain_control[boundary_line_index].counter_repetition = 0;
        auto_gain_control[boundary_line_index].curent_gain = 127;
    }
    auto_gain_control[boundary_line_index].flag_cgange = 0;
}

//
void update_gain_value (uint16_t next_line_index, AUTOGAINCONTROL* auto_gain_control)
{
    if (auto_gain_control[next_line_index].curent_gain != old_gain_value)
    {
        old_gain_value = auto_gain_control[next_line_index].curent_gain;
        xil_printf("Real update gain value: %u\n Next line index: %u\n", old_gain_value, next_line_index);
    }
}

void auto_gain_control(uint16_t start_azimuth, uint16_t *end_azimuth, uint16_t* samples, uint16_t size_of_samples, AUTOGAINCONTROL* auto_gain_control)
{
    uint16_t line_index = start_azimuth / 0x36;
    filling_auto_gain_control_array(samples, size_of_samples, auto_gain_control, line_index);
}