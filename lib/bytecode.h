#ifndef HAMMER_BYTECODE_H_
#define HAMMER_BYTECODE_H_

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "buffer.h"

#define EACH_OPCODE(_)                                                                             \
    _(OP_JUMP, 0x01)      /* jump to relative address unconditionally */                           \
    _(OP_JUMPIF, 0x02)    /* pop a value and jump to relative address if it's `true` */            \
    _(OP_JUMPIFN, 0x03)   /* pop a value and jump to relative address if it's `false` */           \
    _(OP_CALL, 0x04)      /* look up function, push a frame, and jump */                           \
    _(OP_RETURN, 0x05)    /* pop a frame and restore instruction and stack pointers */             \
    _(OP_LOAD, 0x10)      /* load local */                                                         \
    _(OP_STORE, 0x11)     /* store local */                                                        \
    _(OP_DUP, 0x12)       /* duplicate top value on the stack */                                   \
    _(OP_POP, 0x13)       /* remove top value off the stack */                                     \
    _(OP_SWAP, 0x14)      /* swap 2 top operands on the stack */                                   \
    _(OP_PUSHINT, 0x20)   /* push int value to the stack */                                        \
    _(OP_PUSHSTR, 0x21)   /* push string value to the stack */                                     \
    _(OP_PUSHTRUE, 0x22)  /* push the value `true` to the stack */                                 \
    _(OP_PUSHFALSE, 0x23) /* push the value `false` to the stack */                                \
    _(OP_MAKECLS, 0x30)  /* look up function, store N captures, and push closure onto the stack */ \
    _(OP_CALLVAL, 0x31)  /* call value (e.g. closure) with N args */                               \
    _(OP_BIND, 0x32)     /* bind a monadic value to a closure */                                   \
    _(OP_ADD, 0x40)      /* pop two values off the stack and push their sum */                     \
    _(OP_ISTUPLE, 0x50)  /* check if top value on the stack is a N-tuple (don't pop) */            \
    _(OP_TUPLEGET, 0x51) /* push N-th element of tuple on top of the stack (don't pop) */          \
    _(OP_MAKETUPLE, 0x52) /* pop N values and make a tuple */                                      \
    _(OP_MAKELIST, 0x53) /* push `nil` (empty list) and N times: pop 2 values and make a `cons` */ \
    _(OP_ISNIL, 0x54)    /* check if top value on the stack is a `nil` (empty list) (don't pop) */ \
    _(OP_ISCONS, 0x55)   /* check if top value on the stack is a `cons` (list) (don't pop) */      \
    _(OP_UNCONS, 0x56)   /* pop top `cons` and push `tail` and `head` separately */                \
    _(OP_CONCAT, 0x57)   /* pop 2 lists off the stack and push a concatenation */                  \
    _(OP_LOADEXT, 0x80)  /* pop string off the stack and load an external symbol with that name */ \
    _(OP_YIELD, 0xF0)    /* yield control to the effect on the top of the stack */                 \
    _(OP_HALT, 0xFF)     /* stop execution */

#define ENUM_MEMBER(name, byte) name = byte,
typedef enum { EACH_OPCODE(ENUM_MEMBER) } opcode;
#undef ENUM_MEMBER

typedef enum {
    FN_NAMED,
    FN_BLOCK,
    FN_LAMBDA,
    FN_CASE,
    FN_GLOBAL,
} func_type;

typedef struct {
    uint32_t  addr;
    uint8_t   locals;
    uint8_t   args;
    uint8_t   captures;
    func_type type;
    uint32_t  name_offset;
    uint8_t   name_len;
} func_meta;

typedef struct {
    uint8_t addr[4];
    uint8_t locals;
    uint8_t args;
    uint8_t captures;
    uint8_t type;
    uint8_t name_offset[4];
    uint8_t name_len;
} func_meta_bytes;

typedef struct {
    uint16_t         version;
    uint32_t         string_bytes_len;
    const char      *string_bytes;
    uint32_t         funcs_len;
    func_meta_bytes *funcs;
    const uint8_t   *bytecode;
    size_t           bytecode_len;
} program;

#define MAGIC_HAMMER     "HAMMER"
#define BYTECODE_VERSION 0x0001

const char *opcode_name(opcode);

/*
 * `size_t *addr_offset` is set to the offset relative to the buffer start
 * where an `i16be` relative jump address is located. This offset is stored
 * by the compiler to resolve jumps at a later stage.
 */
void bytecode_put_jump(Buffer *, opcode op, size_t *addr_offset);
void bytecode_put_call(Buffer *, uint32_t fnindex);
void bytecode_put_load(Buffer *, uint8_t local);
void bytecode_put_store(Buffer *, uint8_t local);
void bytecode_put_pushint(Buffer *, int64_t value);
void bytecode_put_pushstr(Buffer *, uint32_t offset, uint32_t len);
void bytecode_put_makecls(Buffer *, uint32_t fnindex);
void bytecode_put_callval(Buffer *, uint8_t args);
void bytecode_put_istuple(Buffer *, uint16_t len);
void bytecode_put_tupleget(Buffer *, uint16_t index);
void bytecode_put_maketuple(Buffer *, uint16_t len);
void bytecode_put_makelist(Buffer *, uint16_t len);

void bytecode_put_func_meta(Buffer *, func_meta *);

/*
 * Validates a compiled program and stores pointers to specific sections in `p`.
 * Does not allocate any new buffers.
 */
bool   program_read(program *, const uint8_t *bytes, size_t len);
void   program_func_meta(const program *, uint32_t, func_meta *);
string program_func_name(const program *, func_meta *);
void   program_debug_print(const program *, Buffer *);

#endif // HAMMER_BYTECODE_H_
