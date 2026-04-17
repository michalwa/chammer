#ifndef BYTECODE_H_
#define BYTECODE_H_

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

typedef uint8_t u16be[2];
typedef uint8_t u32be[4];

uint16_t u16be_value(u16be bytes);
uint32_t u32be_value(u32be bytes);

typedef struct {
    // TODO: Add useful trace info
    char _placeholder;
} trace_info;

#define EACH_OPCODE(_) \
    /* _(name, byte, data_type) */ \
    _(OP_TRACE, 1, u16be) /* debug trace with an identifier */ \
    _(OP_PUSHSTR, 2, op_pushstr) /* push string constant */

#define ENUM_MEMBER(name, value, data_type) name = value,
typedef enum { EACH_OPCODE(ENUM_MEMBER) } opcode;
#undef ENUM_MEMBER

typedef struct {
    u32be offset;
    u32be len;
} op_pushstr;

typedef struct {
    u16be      *hammer_version;
    u16be      *trace_table_len;
    trace_info *trace_table;
    u32be      *string_bytes_len;
    char       *string_bytes;
    uint8_t    *bytecode;
    size_t      bytecode_len;
} program;

#define MAGIC_HAMMER "HAMMER"

bool program_read(program *p, uint8_t *bytes, size_t len);

#endif // BYTECODE_H_
