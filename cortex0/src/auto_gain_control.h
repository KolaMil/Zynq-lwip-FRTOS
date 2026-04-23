#ifndef XILINX_MCU_AUTO_GAIN_CONTROL_H
#define XILINX_MCU_AUTO_GAIN_CONTROL_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define NOMINAL_LINES_NUMBER    1200
#define NOMINAL_GAIN_VALUE      127
#define MINIMAL_GAIN_VALUE      100
#define MIN_STEP_GAIN           10
#define ANGULAR_STEP            0x37 // 0.3 grad

typedef struct{
    uint8_t counter_repetition;
    bool flag_change;
    uint8_t current_gain;
} auto_gain_control_t;

typedef struct{
    uint16_t max_permissible_value;
    uint8_t weakening_zone;
    uint8_t number_of_permissible_revolutions;
} auto_gain_control_constants_t;

enum regime
{
    mod0_25_50,
    mod0_25_100,
    mod0_5_50,
    mod0_5_100,
    mod0_75_50,
    mod0_75_100,
    mod1_25_50,
    mod1_25_100,
    mod3_50,
    mod3_100,
    mod6_50,
    mod6_100,
    mod12_50,
    mod12_100,
    mod12_150,
    mod24_100,
    mod24_150,
    mod24_200,
    mod48_150,
    mod48_200
};

auto_gain_control_t* createControlArray();
void cleaningControlArray(auto_gain_control_t* auto_gain_control, uint16_t boundary_line_index);
void updateGainValue (uint16_t next_line_index, auto_gain_control_t* auto_gain_control);
void fillingControlArray(uint16_t* samples, uint16_t size_of_samples, auto_gain_control_t* auto_gain_control, uint16_t line_index);
void autoGainControl(uint16_t start_azimuth, uint16_t* samples, uint16_t size_of_samples, auto_gain_control_t* auto_gain_control);

#endif
