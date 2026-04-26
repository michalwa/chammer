#ifndef BYTES_H_
#define BYTES_H_

#include "buffer.h"

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
