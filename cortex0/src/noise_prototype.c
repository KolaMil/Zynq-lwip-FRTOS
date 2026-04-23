#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define EMA_SHIFT 8U  // Q8: precision 1/256

typedef struct
{
    uint32_t ema_q;        // EMA in fixed-point (Q8)
    uint16_t alpha_num;    // 1
    uint16_t alpha_den;    // 512 => alpha = 1/512 = 0.001953125
    uint32_t packet_count;
    bool initialized;
} noise_tracker_t;

static uint16_t u16_be(const uint8_t *p)
{
    return ((uint16_t)p[0] << 8) | p[1];
}

uint32_t estimate_packet_noise_u16(const uint8_t *data, size_t data_bytes)
{
    uint32_t n = (uint32_t)(data_bytes / 2U);
    if (n == 0U) return 0U;
    uint64_t sum = 0U;
    for (uint32_t i = 0; i < n; i++) 
    {
        sum += u16_be(&data[2U * i]);
    }
    return (uint32_t)(sum / n); // realtime noise (integer)
}

void noise_tracker_update_u16(noise_tracker_t *t, uint32_t packet_noise_u16)
{
    uint32_t x_q = packet_noise_u16 << EMA_SHIFT;
    if (!t->initialized) 
    {
        t->ema_q = x_q;
        t->initialized = true;
    } else 
    {
        int32_t diff = (int32_t)x_q - (int32_t)t->ema_q;
        int32_t delta = (int32_t)((((int64_t)t->alpha_num) * diff) / t->alpha_den);
        t->ema_q = (uint32_t)((int32_t)t->ema_q + delta);
    }
    t->packet_count++;
}

uint32_t noise_tracker_get_ema_u16(const noise_tracker_t *t)
{
    return t->ema_q >> EMA_SHIFT;
}