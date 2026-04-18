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
    _(OP_TRACE, 1) /* debug trace with an identifier */ \
    _(OP_PUSHINT, 2) /* push int constant */ \
    _(OP_PUSHSTR, 3) /* push string constant */ \
    _(OP_ADD, 4) /* builtin binary (+) operation */ \
    _(OP_CALL, 5) /* push frame and jump to instruction */ \
    _(OP_RETURN, 6) /* return from OP_CALL */

#define ENUM_MEMBER(name, byte) name = byte,
typedef enum { EACH_OPCODE(ENUM_MEMBER) } opcode;
#undef ENUM_MEMBER

typedef void *op_void;

typedef struct {
    uint32_t offset;
    uint32_t len;
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

void bytecode_set_u16be(char *, uint16_t);
void bytecode_set_u32be(char *, uint32_t);
void bytecode_set_u64be(char *, uint64_t);

void bytecode_put_u16be(Buffer *, uint16_t);
void bytecode_put_u32be(Buffer *, uint32_t);
void bytecode_put_u64be(Buffer *, uint64_t);

void bytecode_put_trace(Buffer *, uint16_t);
void bytecode_put_pushint(Buffer *, uint64_t value);
void bytecode_put_pushstr(Buffer *, uint32_t offset, uint32_t len);
void bytecode_put_call(Buffer *b);

bool program_read(program *p, uint8_t *bytes, size_t len);

#endif // BYTECODE_H_
