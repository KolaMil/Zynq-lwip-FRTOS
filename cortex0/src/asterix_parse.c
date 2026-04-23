#include "asterix.h"

static inline int is_little_endian(void) {  // if true - little endian
    uint16_t x = 1;
    return *(uint8_t*)&x == 1;
}

// immitation ntohs for us
static inline uint16_t ntohs(uint16_t netshort) {
    if (is_little_endian()) {
        return (netshort >> 8) | (netshort << 8);
    }
    return netshort;
}

// immitation ntohl for us
static inline uint32_t ntohl(uint32_t netlong) {
    if (is_little_endian()) {
        return ((netlong >> 24) & 0x000000FF) |
               ((netlong >> 8)  & 0x0000FF00) |
               ((netlong << 8)  & 0x00FF0000) |
               ((netlong << 24) & 0xFF000000);
    }
    return netlong;
}

// read uint16 in buffer (big-endian)
static inline uint16_t read_u16(const uint8_t **buf) {
    uint16_t val;
    memcpy(&val, *buf, 2);
    *buf += 2;
    return ntohs(val);
}

// read uint32 in buffer (big-endian)
static inline uint32_t read_u32(const uint8_t **buf) {
    uint32_t val;
    memcpy(&val, *buf, 4);
    *buf += 4;
    return ntohl(val);
}

// read uint24 (3 byte) and return in uint32 (little 24 bits)
static inline uint32_t read_u24(const uint8_t **buf) {
    uint32_t val = ((uint32_t)(*buf)[0] << 16) |
                   ((uint32_t)(*buf)[1] << 8)  |
                   ((uint32_t)(*buf)[2]);
    *buf += 3;
    return val;
}

// Read 1 byte
static inline uint8_t read_u8(const uint8_t **buf) {
    return *(*buf)++;
}

bool parse_cat240(void *data, size_t data_len, cat240_message_t *msg) 
{
	if (data_len < 3)
    { 
        xil_printf("Dropped in check data_len!");
        return false;  // Minimum CAT + LEN
    }
    const uint8_t *p = data;

    // 1. Read Head of block
    uint8_t cat = read_u8(&p);
    // xil_printf("CAT: %d\r\n", cat);
    if (cat != 240) 
    {
        xil_printf("Dropped in check type cat240!");
        return false;
    }
    uint16_t block_len = read_u16(&p);
    if (block_len < 3 || block_len > data_len) 
    {
        xil_printf("Dropped in check lenght!");
        return false;
    } // Check lenght

    size_t remaining = block_len - 3; // remains CAT and LEN

    // 2. Read FSPEC (first byte)
    uint8_t fspec1 = read_u8(&p);
    remaining--;
    bool has_fspec2 = (fspec1 & 0x01) != 0;
    uint8_t fspec2 = 0;
    if (has_fspec2) 
    {
        if (remaining == 0) 
        {
            xil_printf("Dropped in check fspec lenght!");
            return false;
        }
        fspec2 = read_u8(&p);
        remaining--;
        // second byte of FX always 0
        if (fspec2 & 0x01)
        {
            return false; // Report: FX not 0
            xil_printf("Dropped in check fspec!");
        }
    }
    // Clear struct of msg
    memset(msg, 0, sizeof(*msg));

    // 3. Check step by step FRN 1..7 with first byte FSPEC
    //    bit 7 (0x80) -> FRN1, bit 6 (0x40) -> FRN2, ... bit 1 (0x02) -> FRN7
    //    bit 0 (0x01) alredy used for FX

    // FRN1 (I240/010) - always must to be
    if (fspec1 & 0x80) 
    {
        if (remaining < 2) return false;
        msg->data_source.sac = read_u8(&p);
        msg->data_source.sic = read_u8(&p);
        remaining -= 2;
    } else {
        return false; // necessarily SAC/SIC 
    }

    // FRN2 (I240/000)
    if (fspec1 & 0x40) 
    {
        if (remaining < 1) return false;
        msg->message_type = read_u8(&p);
        remaining -= 1;
    } else {
        return false; // Message Type necessarily
    }

    // FRN3 (I240/020) - necessarily for Video Message, optional for Summary? Specifikation: Video Message necessarily.
    if (fspec1 & 0x20) 
    {
        if (remaining < 4) return false;
        msg->has_video_header = true;
        msg->msg_index = read_u32(&p);
        remaining -= 4;
    }

    // FRN4 (I240/030) - necessarily for Summary
    if (fspec1 & 0x10) 
    {
        if (remaining < 1) return false;
        msg->has_summary = true;
        uint8_t rep = read_u8(&p);
        remaining -= 1;
        if (remaining < rep) return false;
        msg->summary.rep = rep;
        if (rep > sizeof(msg->summary.text)-1) rep = sizeof(msg->summary.text)-1;
        memcpy(msg->summary.text, p, rep);
        msg->summary.text[rep] = '\0';
        p += rep;
        remaining -= rep;
    }

    // FRN5 (I240/040)
    if (fspec1 & 0x08) {
        if (remaining < 12) return false;
        msg->has_header_nano = true;
        msg->header_nano.start_az = read_u16(&p);
        msg->header_nano.end_az   = read_u16(&p);
        msg->header_nano.start_rg = read_u32(&p);
        msg->header_nano.cell_dur = read_u32(&p); // nanosec
        remaining -= 12;
    }

    // FRN6 (I240/041)
    if (fspec1 & 0x04) 
    {
        if (remaining < 12) return false;
        msg->has_header_femto = true;
        msg->header_femto.start_az = read_u16(&p);
        msg->header_femto.end_az   = read_u16(&p);
        msg->header_femto.start_rg = read_u32(&p);
        msg->header_femto.cell_dur = read_u32(&p); // femtosec
        remaining -= 12;
    }

    // FRN7 (I240/048)
    if (fspec1 & 0x02) 
    {
        if (remaining < 2) return false;
        msg->has_resolution = true;
        uint16_t res_comp = read_u16(&p);
        msg->resolution.compression = (res_comp >> 15) & 1; // bit 16 (older) - C
        msg->resolution.reserved    = (res_comp >> 8) & 0x7F;
        msg->resolution.resolution  = res_comp & 0xFF;
        remaining -= 2;
    }

    // 4. If have second byte FSPEC, check FRN 8..14
    if (has_fspec2) 
    {
        // FRN8 (I240/049)
        if (fspec2 & 0x80) 
        {
            if (remaining < 5) return false;
            msg->has_counters = true;
            msg->counters.nb_vb = read_u16(&p);
            msg->counters.nb_cells = read_u24(&p);
            remaining -= 5;
        }

        // FRN9 (I240/050)
        if (fspec2 & 0x40) 
        {
            if (remaining < 1) return false;
            msg->has_video_block = true;
            msg->video_block.type = VIDEO_BLOCK_LOW;
            uint8_t rep = read_u8(&p);
            remaining -= 1;
            size_t block_bytes = rep * 4;
            if (remaining < block_bytes) return false;
            msg->video_block.rep = rep;
            msg->video_block.data = p;
            msg->video_block.data_bytes = block_bytes;
            p += block_bytes;
            remaining -= block_bytes;
        }

        // FRN10 (I240/051)
        if (fspec2 & 0x20) 
        {
            if (remaining < 1) return false;
            msg->has_video_block = true;
            msg->video_block.type = VIDEO_BLOCK_MEDIUM;
            uint8_t rep = read_u8(&p);
            remaining -= 1;
            size_t block_bytes = rep * 64;
            if (remaining < block_bytes) return false;
            msg->video_block.rep = rep;
            msg->video_block.data = p;
            msg->video_block.data_bytes = block_bytes;
            // p += block_bytes;
            p += msg->counters.nb_vb;
            remaining -= block_bytes;
        }

        // FRN11 (I240/052)
        if (fspec2 & 0x10) 
        {
            if (remaining < 1) return false;
            msg->has_video_block = true;
            msg->video_block.type = VIDEO_BLOCK_HIGH;
            uint8_t rep = read_u8(&p);
            remaining -= 1;
            size_t block_bytes = rep * 256;
            if (remaining < block_bytes) return false;
            msg->video_block.rep = rep;
            msg->video_block.data = p;
            msg->video_block.data_bytes = block_bytes;
            p += block_bytes;
            remaining -= block_bytes;
        }

        // FRN12 (I240/140)
        if (fspec2 & 0x08) 
        {
            if (remaining < 3) return false;
            msg->has_time = true;
            msg->time_of_day = read_u24(&p);
            remaining -= 3;
        }

        // FRN13 (RE) - skip her as opaque data
        if (fspec2 & 0x04) 
        {
            if (remaining < 1) return false;
            msg->has_re = true;
            uint8_t rep = read_u8(&p);
            remaining -= 1;
            if (remaining < rep) return false;
            msg->re_len = rep;
            p += rep;
            remaining -= rep;
        }

        // FRN14 (SP)
        if (fspec2 & 0x02) 
        {
            if (remaining < 1) return false;
            msg->has_sp = true;
            uint8_t rep = read_u8(&p);
            remaining -= 1;
            if (remaining < rep) return false;
            msg->sp_len = rep;
            p += rep;
            remaining -= rep;
        }
    }

    // After check all field all bytes must be 0
    if (remaining != 0) {
        // There may be an alignment or an error; the specification says that there should be no extra bytes.
    }

    return true;
}
