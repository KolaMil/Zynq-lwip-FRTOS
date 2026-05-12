#include "noise.h"

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

void noise_tracker_update_smoothing(noise_tracker_t *tracker, uint32_t packet_noise_u16)
{
    if (packet_noise_u16 > tracker->max_value_noise)
    {
        return;
    }
    
    uint32_t x_q = packet_noise_u16 << EMA_SHIFT;
    if (!tracker->initialized) 
    {
        tracker->ema_q = x_q;
        tracker->initialized = true;
        tracker->max_value_noise = NOISE_MAX_DEFAULT_U16;
    } else 
    {
        int32_t diff = (int32_t)x_q - (int32_t)tracker->ema_q;
        int32_t delta;
        if (diff > 0 && (uint64_t)diff > (((uint64_t)tracker->ema_q * 70/100) + (uint64_t)tracker->ema_q))
        {
            delta = (int32_t)((((int64_t)tracker->alpha_num_slow) * diff) / tracker->alpha_den_slow);
        } else
        {
            delta = (int32_t)((((int64_t)tracker->alpha_num) * diff) / tracker->alpha_den);
        }
        
        tracker->ema_q = (uint32_t)((int32_t)tracker->ema_q + delta);
    }
    tracker->packet_count++;
}

uint32_t get_noise_per_round(noise_tracker_t *tracker, uint32_t *packet_noise_u16, size_t size)
{
    if (packet_noise_u16 == NULL || size == 0 || tracker == NULL)
    {
        return;
    }
    
    uint64_t sum = 0;
    for (size_t i = 0; i < size; i++)
    {
        sum += packet_noise_u16[i];
    }
    tracker->ema_q = (uint32_t)(sum / size);

    if (!tracker->initialized)
    {
        // tracker->ema_q = mean;
        tracker->initialized = true;
        // tracker->max_value_noise = NOISE_MAX_DEFAULT_U16;
    }
    return tracker->ema_q;
}

uint32_t get_noise_per_3rounds(uint32_t *packet_noise_u16, size_t size)
{
    if (packet_noise_u16 == NULL || size == 0)
    {
        return;
    }
    
    uint64_t sum = 0;
    for (size_t i = 0; i < size; i++)
    {
        sum += packet_noise_u16[i];
    }

    return (uint32_t)sum/size;
}

// uint32_t noise_tracker_get_ema_u16(const noise_tracker_t *tracker)
// {
//     return tracker->ema_q >> EMA_SHIFT;
// }
