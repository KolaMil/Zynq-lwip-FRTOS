#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define EMA_SHIFT               8U  // Q8: precision 1/256
#define NOISE_MAX_DEFAULT_U16   30000

typedef struct
{
    uint16_t noise_value_per_packets[1200];
    uint16_t middle_noise_val_per_3round[3];

    uint32_t packet_count;
    uint32_t ema_q;                             // Noise level in fixed-ring
    uint16_t middle_noise_val_per_round;
    uint16_t alpha_num;                         // 1
    uint16_t alpha_den;                         // 512 => alpha = 1/512 = 0.001953125
    uint16_t alpha_num_slow;                    // медленный коэффициент для резких скачков вверх
    uint16_t alpha_den_slow;
    uint16_t max_value_noise;
    bool initialized;
} noise_tracker_t;

uint32_t estimate_packet_noise_u16(const uint8_t *data, size_t data_bytes);
void noise_tracker_update_smoothing(noise_tracker_t *t, uint32_t packet_noise_u16);
uint32_t get_noise_per_round(noise_tracker_t *tracker, uint32_t *packet_noise_u16, size_t size);
uint32_t get_noise_per_3rounds(uint32_t *packet_noise_u16, size_t size);
// uint32_t noise_tracker_get_ema_u16(const noise_tracker_t *t);