#ifndef HAMMER_BYTES_H_
#define HAMMER_BYTES_H_

#include <inttypes.h>

#include "buffer.h"

#define CHECKED_INT_CAST(type, min, max, value)                                             \
    (((min) <= (value) && (value) <= (max))                                                 \
         ? (type)(value)                                                                    \
         : (panic(#value " cast to " #type " with overflow: %lld", (long long)(value)), 0))

#define CHECKED_U8(value)  CHECKED_INT_CAST(uint8_t, 0, UINT8_MAX, value)
#define CHECKED_U16(value) CHECKED_INT_CAST(uint16_t, 0, UINT16_MAX, value)
#define CHECKED_U32(value) CHECKED_INT_CAST(uint32_t, 0, UINT32_MAX, value)
#define CHECKED_U64(value) CHECKED_INT_CAST(uint64_t, 0, UINT64_MAX, value)
#define CHECKED_I8(value)  CHECKED_INT_CAST(int8_t, INT8_MIN, INT8_MAX, value)
#define CHECKED_I16(value) CHECKED_INT_CAST(int16_t, INT16_MIN, INT16_MAX, value)
#define CHECKED_I32(value) CHECKED_INT_CAST(int32_t, INT32_MIN, INT32_MAX, value)
#define CHECKED_I64(value) CHECKED_INT_CAST(int64_t, INT64_MIN, INT64_MAX, value)

uint16_t read_u16be(const uint8_t **);
uint32_t read_u32be(const uint8_t **);
uint64_t read_u64be(const uint8_t **);
int16_t  read_i16be(const uint8_t **);
int32_t  read_i32be(const uint8_t **);
int64_t  read_i64be(const uint8_t **);

void write_u16be(uint8_t **, uint16_t);
void write_u32be(uint8_t **, uint32_t);
void write_u64be(uint8_t **, uint64_t);
void write_i16be(uint8_t **, int16_t);
void write_i32be(uint8_t **, int32_t);
void write_i64be(uint8_t **, int64_t);

void buffer_put_u16be(Buffer *, uint16_t);
void buffer_put_u32be(Buffer *, uint32_t);
void buffer_put_u64be(Buffer *, uint64_t);
void buffer_put_i16be(Buffer *, int16_t);
void buffer_put_i32be(Buffer *, int32_t);
void buffer_put_i64be(Buffer *, int64_t);

#endif // HAMMER_BYTES_H_
