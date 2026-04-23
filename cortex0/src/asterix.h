#ifndef SRC_ASTERIX_H_
#define SRC_ASTERIX_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_VIDEO_BLOCK_LOW     1020
#define MAX_VIDEO_BLOCK_MEDIUM  16320
#define MAX_VIDEO_BLOCK_HIGH    65024
#define HEADER_SIZE             32  // with rep, lenght, category
#define TIME_SIZE               3
#define AVERAGE_DIFFERENCE_AZ   0x00DA  // 0x00DA is a average difference between start_az and end_az

// I240/010
typedef struct {
    uint8_t sac;
    uint8_t sic;
} cat240_data_source_identifier_t;

// I240/000
typedef uint8_t cat240_message_type_t;  // 1 or 2

// I240/020
// typedef uint32_t cat240_video_record_header_t;  // MSG_INDEX

// I240/030
typedef struct {
    uint8_t rep;                    // count of symbols
    char    text[256];              // max lenght
} cat240_video_summary_t;

// I240/040 I240/041 (general struct, have different in CELL_DUR)
typedef struct {
    uint16_t start_az;              // LSB = 360/2^16
    uint16_t end_az;
    uint32_t start_rg;              // number of start cell
    uint32_t cell_dur;              // duration (nanosec Nano)
} cat240_video_header_t;

// I240/048
typedef struct {
    uint8_t compression : 1;        // bit 16 (C)
    uint8_t reserved    : 7;        // bits 15-9 reserved
    uint8_t resolution;             // bits 8-1 (RES)
} cat240_resolution_indicator_t;

// I240/049
typedef struct {
    uint16_t nb_vb;              // count of valid bytes in block
    uint32_t nb_cells;           // count of valid cells (3 byte, save in uint32_t)
} cat240_counters_t;

typedef enum {
    VIDEO_BLOCK_LOW,
    VIDEO_BLOCK_MEDIUM,
    VIDEO_BLOCK_HIGH
} cat240_block_type_t;

typedef struct {
    cat240_block_type_t type;
    uint8_t             rep;  // counter of 4-party values
    const uint8_t       *data;
    size_t              data_bytes;
} cat240_video_block_t;

// I240/140
typedef uint32_t cat240_time_of_day_t;  // 24 bits


typedef struct {
    cat240_data_source_identifier_t data_source;
    cat240_message_type_t           message_type;

    bool                            has_video_header;      // FRN3
    uint32_t                        msg_index;

    bool                            has_summary;           // FRN4
    cat240_video_summary_t          summary;

    bool                            has_header_nano;       // FRN5
    cat240_video_header_t           header_nano;
    bool                            has_header_femto;      // FRN6
    cat240_video_header_t           header_femto;

    bool                            has_resolution;        // FRN7
    cat240_resolution_indicator_t   resolution;

    bool                            has_counters;          // FRN8
    cat240_counters_t               counters;

    bool                            has_video_block;       // FRN9/10/11
    cat240_video_block_t            video_block;

    bool                            has_time;              // FRN12
    cat240_time_of_day_t            time_of_day;

    uint8_t                         check_sum;
    
    bool                            has_re;
    size_t                          re_len;                // lenght RE
    bool                            has_sp;
    size_t                          sp_len;
} cat240_message_t;

typedef struct
{
    uint8_t *header_data;
    size_t header_size;
	uint8_t *video_data;
    size_t data_size;
	uint16_t start_az;
    uint16_t end_az;
    uint8_t time_per_bytes[3];
    size_t time_size;
} cat240_storage_t;

bool parse_cat240(void *data, size_t data_len, cat240_message_t *msg);

#endif /* SRC_ASTERIX_H_ */
