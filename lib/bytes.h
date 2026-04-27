#ifndef BYTES_H_
#define BYTES_H_

#include "buffer.h"

#define CHECKED_UINT_CAST(type, max, value)                                                 \
    ((0 <= (value) && (value) <= max)                                                       \
         ? (type)(value)                                                                    \
         : (panic(#value " cast to " #type " with overflow: %lld", (long long)(value)), 0))

#define CHECKED_U8(value)  CHECKED_UINT_CAST(uint8_t, UINT8_MAX, value)
#define CHECKED_U16(value) CHECKED_UINT_CAST(uint16_t, UINT16_MAX, value)
#define CHECKED_U32(value) CHECKED_UINT_CAST(uint32_t, UINT32_MAX, value)

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

#endif // BYTES_H_
