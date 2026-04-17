#ifndef BYTECODE_H_
#define BYTECODE_H_

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "buffer.h"

typedef uint8_t u16be[2];
typedef uint8_t u32be[4];
typedef uint8_t u64be[8];

typedef struct {
    // TODO: Add useful trace info
    char _placeholder;
} trace_info;

#define EACH_OPCODE(_) \
    /* _(name, byte, data_size) */ \
    _(OP_TRACE, 1, sizeof(u16be)) /* debug trace with an identifier */ \
    _(OP_PUSHINT, 2, sizeof(u64be)) /* push int constant */ \
    _(OP_PUSHSTR, 3, sizeof(op_pushstr)) /* push string constant */ \
    _(OP_ADD, 4, 0) /* builtin binary (+) operation */

#define ENUM_MEMBER(name, byte, data_size) name = byte,
typedef enum { EACH_OPCODE(ENUM_MEMBER) } opcode;
#undef ENUM_MEMBER

typedef struct {
    u32be offset;
    u32be len;
} op_pushstr;

typedef struct {
    u16be      *version;
    u16be      *trace_table_len;
    trace_info *trace_table;
    u32be      *string_bytes_len;
    char       *string_bytes;
    uint8_t    *bytecode;
    size_t      bytecode_len;
} program;

#define MAGIC_HAMMER "HAMMER"
#define BYTECODE_VERSION 0x0001

uint16_t u16be_value(u16be);
uint32_t u32be_value(u32be);
uint64_t u64be_value(u64be);

void buffer_write_u16be(Buffer *, uint16_t);
void buffer_write_u32be(Buffer *, uint32_t);
void buffer_write_u64be(Buffer *, uint64_t);

size_t opcode_data_size(opcode);

bool program_read(program *p, uint8_t *bytes, size_t len);

#endif // BYTECODE_H_
