#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define EMA_SHIFT               8U  // Q8: precision 1/256
#define NOISE_MAX_DEFAULT_U16   40000

typedef struct
{
    bool initialized;
    uint16_t alpha_num;         // 1
    uint16_t alpha_den;         // 512 => alpha = 1/512 = 0.001953125
    uint16_t alpha_num_slow;    // медленный коэффициент для резких скачков вверх
    uint16_t alpha_den_slow;
    uint16_t max_value_noise;
    uint32_t ema_q;             // EMA in fixed-point (Q8)
    uint32_t packet_count;
} noise_tracker_t;

uint32_t estimate_packet_noise_u16(const uint8_t *data, size_t data_bytes);
void noise_tracker_update_u16(noise_tracker_t *t, uint32_t packet_noise_u16);
uint32_t noise_tracker_get_ema_u16(const noise_tracker_t *t);