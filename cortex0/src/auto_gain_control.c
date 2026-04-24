#include "auto_gain_control.h"

auto_gain_control_constants_t autogain_control_constants[20] = {
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

auto_gain_control_t template = 
{
	.counter_repetition = 0,
	.flag_change = false,
	.current_gain = NOMINAL_GAIN_VALUE
};

static 	uint8_t old_gain_value = 127;

auto_gain_control_t* createControlArray()
{
    auto_gain_control_t *auto_gain_control = malloc(sizeof(auto_gain_control_t) * NOMINAL_LINES_NUMBER);
    if (auto_gain_control == NULL) return NULL;
    for (uint16_t i = 0; i < NOMINAL_LINES_NUMBER; i++)
    {
        auto_gain_control[i] = template;
    }
    return auto_gain_control;
}

void cleaningControlArray(auto_gain_control_t* auto_gain_control, uint16_t boundary_line_index)
{
    if (auto_gain_control[boundary_line_index].flag_change)
    {
        auto_gain_control[boundary_line_index].counter_repetition = 0;
        auto_gain_control[boundary_line_index].current_gain = NOMINAL_GAIN_VALUE;
    }
    auto_gain_control[boundary_line_index].flag_change = false;
}

void updateGainValue (uint16_t next_line_index, auto_gain_control_t* auto_gain_control)
{
    static uint8_t data_to_send_PL[2] = { (uint8_t)127, 0xC3 };
    if (auto_gain_control[next_line_index].current_gain != old_gain_value)
    {
        old_gain_value = auto_gain_control[next_line_index].current_gain;
        data_to_send_PL[0] = old_gain_value;
        send_to_PL(data_to_send_PL, 2);
    }
}

void fillingControlArray(uint16_t* samples, uint16_t size_of_samples, auto_gain_control_t* auto_gain_control, uint16_t line_index)  // search blind value
{
    for (uint16_t sample_index = 0; sample_index < size_of_samples; sample_index++)
    {
        if (samples[sample_index] > autogain_control_constants[0].max_permissible_value)
        {
            for (int16_t weakening_line_index = autogain_control_constants[0].weakening_zone * (-1); weakening_line_index <= autogain_control_constants[0].weakening_zone; weakening_line_index++)
            {
                uint16_t real_weakening_line_index = ((line_index + NOMINAL_LINES_NUMBER) + weakening_line_index) % NOMINAL_LINES_NUMBER;
                if (!auto_gain_control[real_weakening_line_index].flag_change)
                {
                    auto_gain_control[real_weakening_line_index].counter_repetition++;
                    if (auto_gain_control[real_weakening_line_index].counter_repetition > autogain_control_constants[0].number_of_permissible_revolutions)
                    {
                        if (auto_gain_control[real_weakening_line_index].current_gain <= MINIMAL_GAIN_VALUE + MIN_STEP_GAIN)
                        {
                            auto_gain_control[real_weakening_line_index].current_gain = MINIMAL_GAIN_VALUE;
                        }
                        else
                        {
                            auto_gain_control[real_weakening_line_index].current_gain -= MIN_STEP_GAIN;
                        }
                    }
                    auto_gain_control[real_weakening_line_index].flag_change = true;
                }
            }
            break;
        }
    }
    cleaningControlArray(auto_gain_control, ((line_index + NOMINAL_LINES_NUMBER) - (autogain_control_constants[0].weakening_zone + 1)) % NOMINAL_LINES_NUMBER);
    updateGainValue((line_index + 1) % NOMINAL_LINES_NUMBER, auto_gain_control);
}

void autoGainControl(uint16_t start_azimuth, uint16_t* samples, uint16_t size_of_samples, auto_gain_control_t* auto_gain_control)
{
    uint16_t line_index = start_azimuth / ANGULAR_STEP;
    fillingControlArray(samples, size_of_samples, auto_gain_control, line_index);
}

bool convertTo16(uint8_t* massive, size_t size, uint16_t* converted_massive)
{
    if (size % 2 != 0 || size == 0 || converted_massive == NULL)
    {
        return false;
    }

    for (size_t i = 0, k = 0; i < size; i += 2, k++)
    {
        converted_massive[k] = massive[i] * 0x100 + massive[i + 1];
    }

    return true;
}
