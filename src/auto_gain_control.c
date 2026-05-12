//
// Created by nmil on 13.04.2026.
//
#include "auto_gain_control.h"

#include "xil_printf.h"
static uint8_t old_gain_value = 127;

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

/*-----------------------------------------------------------*/
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

/*-----------------------------------------------------------*/
static bool flag_first_disckret = true;
uint32_t average_values_of_the_ruler[1200] = {0};
uint32_t average_value_per_revolution[3] = {0};
uint32_t average_value_over_three_revolutions = 0;
uint8_t counter_to_3 = 0;
static uint16_t discrete_value;
void filling_auto_gain_control_array(struct pbuf *p, uint16_t size_of_samples, AUTOGAINCONTROL* auto_gain_control, uint16_t line_index)
{
    static uint8_t kursor;
    static uint32_t predel_na_shag;
    static uint32_t total_len;
    total_len = p->tot_len;
    kursor = 74;
    for (struct pbuf *q = p; q != NULL; q = q->next)
    {
        if (total_len > 1480)
        {
            total_len -= 1480;
            predel_na_shag = 1480 / 2;
        }
        else
        {
            predel_na_shag = total_len % 1480 / 2;
        }
        for (uint16_t sample_index = 0; sample_index < size_of_samples; sample_index++)
        {
            discrete_value = ((uint8_t *)q->payload + kursor)[sample_index * 2] * 0x100 + ((uint8_t *)q->payload + kursor)[sample_index * 2 + 1];
            if (flag_first_disckret)
            {
                average_values_of_the_ruler[line_index] = discrete_value;
                flag_first_disckret = false;
            }
            else
            {
                average_values_of_the_ruler[line_index] += discrete_value;
            }
            if (discrete_value > autogain_control_constants[0].max_permissible_value)
            {
                for (int16_t weakening_line_index = autogain_control_constants[0].weakening_zone * (-1); weakening_line_index <= autogain_control_constants[0].weakening_zone; weakening_line_index++)
                {
                    int16_t real_weakening_line_index = ((line_index + NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION) + weakening_line_index) % NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION;
                    if (!auto_gain_control[real_weakening_line_index].flag_cgange)
                    {
                        auto_gain_control[real_weakening_line_index].counter_repetition++;
                        if (auto_gain_control[real_weakening_line_index].counter_repetition > autogain_control_constants[0].number_of_permissible_revolutions)
                        {
                            if (auto_gain_control[real_weakening_line_index].curent_gain - 10 <= MINIMAL_GAIN_VALUE)
                            {
                                auto_gain_control[real_weakening_line_index].curent_gain = MINIMAL_GAIN_VALUE;
                            }
                            else
                            {
                                auto_gain_control[real_weakening_line_index].curent_gain -= 10;
                            }
                        }
                        auto_gain_control[real_weakening_line_index].flag_cgange = 1;
                    }
                }
                // break;
                goto target;
            }
        }
        kursor = 0;
    }
    target :
    average_values_of_the_ruler[line_index] = average_values_of_the_ruler[line_index] / size_of_samples;
    xil_printf("average_values_of_the_ruler[line_index] : %2x\r\n", average_values_of_the_ruler[line_index]);
    if (line_index == 98)
    {
        for (uint16_t i = 0; i <= 98; i++)
        {
            average_value_per_revolution[counter_to_3] += average_values_of_the_ruler[i];
        }
        average_value_per_revolution[counter_to_3] /= 98;
        xil_printf("~~~~ average_value_per_revolution : %2x\r\n", average_value_per_revolution[0]);
        if (counter_to_3 != 2)
        {
            counter_to_3++;
        }
        else {
            average_value_over_three_revolutions = (average_value_per_revolution[0] + average_value_per_revolution[1] + average_value_per_revolution[2]) / 3;
            xil_printf("#### average_value_over_three_revolutions : %2x\r\n", average_value_over_three_revolutions);
            average_value_per_revolution[0] = average_value_per_revolution[1];
            average_value_per_revolution[1] = average_value_per_revolution[2];
        }
    }
    flag_first_disckret = false;
    cleaning_auto_gain_control_array(auto_gain_control, ((line_index + NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION) - (autogain_control_constants[0].weakening_zone + 1)) % NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION);
    update_gain_value((line_index + 1) % NOMINAL_NUMBER_OF_LINES_PER_REVOLUTION, auto_gain_control);
}

/*-----------------------------------------------------------*/
void cleaning_auto_gain_control_array(AUTOGAINCONTROL* auto_gain_control, uint16_t boundary_line_index)
{
    if (!auto_gain_control[boundary_line_index].flag_cgange)
    {
        auto_gain_control[boundary_line_index].counter_repetition = 0;
        auto_gain_control[boundary_line_index].curent_gain = 127;
    }
    auto_gain_control[boundary_line_index].flag_cgange = 0;
}

/*-----------------------------------------------------------*/
void update_gain_value (uint16_t next_line_index, AUTOGAINCONTROL* auto_gain_control)
{
    static uint8_t data_to_send_PL[2] = { (uint8_t)127, 0xC3 };
    if (auto_gain_control[next_line_index].curent_gain != old_gain_value)
    {
        old_gain_value = auto_gain_control[next_line_index].curent_gain;
        data_to_send_PL[0] = old_gain_value;
        send_to_PL(data_to_send_PL, 2); //
    }
}

/*-----------------------------------------------------------*/
void auto_gain_control(struct pbuf *p, AUTOGAINCONTROL* auto_gain_control)
{
    static uint32_t line_index;
    static uint16_t size_of_samples;
    static uint16_t r = 3;
    if (r == 0.25){
        // 54 55
    }
    else if (r == 3) {
        // 40 41
        // mode 48 :  40 41
        // sampels 56 55 54
        line_index = (((uint8_t *)p->payload)[40] * 0x100 + ((uint8_t *)p->payload)[41]) / 0x36;
        size_of_samples = (((uint8_t *)p->payload)[56] * 0x10000 + ((uint8_t *)p->payload)[57] * 0x100 + ((uint8_t *)p->payload)[58]) * 2;
    }
    line_index = (((uint8_t *)p->payload + 54)[0] * 0x100 + ((uint8_t *)p->payload + 55)[0]) / 0x36;
    xil_printf("line_index %u\r\n", line_index);
    size_of_samples = (((uint8_t *)p->payload + 70)[0] * 0x10000 + ((uint8_t *)p->payload + 71)[0] * 0x100 + ((uint8_t *)p->payload + 72)[0]);
    filling_auto_gain_control_array(p, size_of_samples, auto_gain_control, line_index);
}