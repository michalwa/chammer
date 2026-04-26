#include "bytes.h"

#define READ_BE_BYTES(t, b) \
    do { \
        t v = 0; \
        for (intptr_t i = sizeof(t) - 1; i >= 0; i--) \
            v = (v << 8) | *(*b)++; \
        return v; \
    } while (0)

inline uint16_t read_u16be(const uint8_t **b) {
    READ_BE_BYTES(uint16_t, b);
}

inline uint32_t read_u32be(const uint8_t **b) {
    READ_BE_BYTES(uint32_t, b);
}

inline uint64_t read_u64be(const uint8_t **b) {
    READ_BE_BYTES(uint64_t, b);
}

inline int16_t read_i16be(const uint8_t **b) {
    READ_BE_BYTES(int16_t, b);
}

inline int32_t read_i32be(const uint8_t **b) {
    READ_BE_BYTES(int32_t, b);
}

inline int64_t read_i64be(const uint8_t **b) {
    READ_BE_BYTES(int64_t, b);
}

#undef READ_BE_BYTES

#define WRITE_BE_BYTES(t, b, v) \
    do { \
        for (intptr_t i = sizeof(t) - 1; i >= 0; i--) \
            *(*b)++ = (v >> (i << 3)) & 0xFF; \
    } while (0)

inline void write_u16be(uint8_t **b, uint16_t v) {
    WRITE_BE_BYTES(uint16_t, b, v);
}

inline void write_u32be(uint8_t **b, uint32_t v) {
    WRITE_BE_BYTES(uint32_t, b, v);
}

inline void write_u64be(uint8_t **b, uint64_t v) {
    WRITE_BE_BYTES(uint64_t, b, v);
}

inline void write_i16be(uint8_t **b, int16_t v) {
    WRITE_BE_BYTES(int16_t, b, v);
}

inline void write_i32be(uint8_t **b, int32_t v) {
    WRITE_BE_BYTES(int32_t, b, v);
}

inline void write_i64be(uint8_t **b, int64_t v) {
    WRITE_BE_BYTES(int64_t, b, v);
}

#undef WRITE_BE_BYTES

inline void buffer_put_u16be(Buffer *b, uint16_t v) {
    uint8_t *c = (uint8_t *)buffer_alloc(b, sizeof(uint16_t));
    write_u16be(&c, v);
}

inline void buffer_put_u32be(Buffer *b, uint32_t v) {
    uint8_t *c = (uint8_t *)buffer_alloc(b, sizeof(uint32_t));
    write_u32be(&c, v);
}

inline void buffer_put_u64be(Buffer *b, uint64_t v) {
    uint8_t *c = (uint8_t *)buffer_alloc(b, sizeof(uint64_t));
    write_u64be(&c, v);
}

inline void buffer_put_i16be(Buffer *b, int16_t v) {
    uint8_t *c = (uint8_t *)buffer_alloc(b, sizeof(int16_t));
    write_i16be(&c, v);
}

inline void buffer_put_i32be(Buffer *b, int32_t v) {
    uint8_t *c = (uint8_t *)buffer_alloc(b, sizeof(int32_t));
    write_i32be(&c, v);
}

inline void buffer_put_i64be(Buffer *b, int64_t v) {
    uint8_t *c = (uint8_t *)buffer_alloc(b, sizeof(int64_t));
    write_i64be(&c, v);
}
