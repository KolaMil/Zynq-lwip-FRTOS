/*
 ============================================================================
 Name        : ttc_timer.h
 Author      :
 Version     :
 Description : Inc for ttc_timer.c
 ============================================================================
 */

#ifndef INT_QUEUE_TIMER_H
#define INT_QUEUE_TIMER_H

#define TTC_TIMER_CHANNEL_1 0
#define TTC_TIMER_CHANNEL_2 1
#define TTC_TIMER_CHANNEL_3 2

void vInitialiseTimer(void);
void start_stop_ttc_timer(uint8_t timer_ch_id, uint8_t flag_start_stop);
uint16_t get_ttc_counter_value(uint8_t timer_ch_id);

#endif

