#ifndef BYTECODE_H_
#define BYTECODE_H_

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "buffer.h"

/*
 * Some opcodes expect extra bytes with arguments. Consult the `bytecode_put_*`
 * functions for specification.
 */
#define EACH_OPCODE(_)                                                                             \
    _(OP_JUMP, 0x01)      /* jump to instruction unconditionally */                                \
    _(OP_JUMPIF, 0x02)    /* pop a value and jump to instruction if it's `true` */                 \
    _(OP_JUMPIFN, 0x03)   /* pop a value and jump to instruction if it's `false` */                \
    _(OP_CALL, 0x04)      /* push a function frame with N locals and jump to instruction */        \
    _(OP_RETURN, 0x05)    /* return from OP_CALL */                                                \
    _(OP_LOAD, 0x10)      /* load local */                                                         \
    _(OP_STORE, 0x11)     /* store local */                                                        \
    _(OP_DUP, 0x12)       /* duplicate top value on the stack */                                   \
    _(OP_POP, 0x13)       /* remove top value off the stack */                                     \
    _(OP_PUSHINT, 0x20)   /* push int value to the stack */                                        \
    _(OP_PUSHSTR, 0x21)   /* push string value to the stack */                                     \
    _(OP_PUSHTRUE, 0x22)  /* push the value `true` to the stack */                                 \
    _(OP_PUSHFALSE, 0x23) /* push the value `false` to the stack */                                \
    _(OP_MAKECLS, 0x30)   /* make closure from captures, N args and instruction address */         \
    _(OP_CALLCLS, 0x31)   /* call closure with N args */                                           \
    _(OP_ADD, 0x40)       /* pop two values off the stack and push their sum */                    \
    _(OP_ISTUPLE, 0x50)   /* check if top value on the stack is a N-tuple (don't pop) */           \
    _(OP_TUPLEGET, 0x51)  /* push N-th element of tuple on top of the stack (don't pop) */         \
    _(OP_MAKETUPLE, 0x52) /* pop N values and make a tuple */                                      \
    _(OP_MAKELIST, 0x53) /* push `nil` (empty list) and N times: pop 2 values and make a `cons` */ \
    _(OP_ISNIL, 0x54)    /* check if top value on the stack is a `nil` (empty list) (don't pop) */ \
    _(OP_ISCONS, 0x55)   /* check if top value on the stack is a `cons` (list) (don't pop) */      \
    _(OP_UNCONS, 0x56)   /* pop top `cons` and push `tail` and `head` separately */                \
    _(OP_CONCAT, 0x57)   /* pop 2 lists off the stack and push a concatenation */                  \
    _(OP_LOADEXT, 0x80)  /* pop string off the stack and load an external symbol with that name */ \
    _(OP_HALT, 0xFF)     /* stop execution */

#define ENUM_MEMBER(name, byte) name = byte,
typedef enum { EACH_OPCODE(ENUM_MEMBER) } opcode;
#undef ENUM_MEMBER

typedef struct {
    uint32_t addr;
    uint8_t  locals;
    uint8_t  args;
} func_meta;

typedef struct {
    uint8_t addr[4];
    uint8_t locals;
    uint8_t args;
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

const char *op_name(opcode);

/*
 * The following functions should be used to generate instructions which expect
 * arguments. For 0-arg instructions, `buffer_putc` can be used.
 *
 * `size_t *addr_offset` is set to the offset relative to the buffer start
 * where an `u32be` instruction address is located. This offset is stored
 * by the compiler to resolve jumps at a later stage.
 */

void bytecode_put_jump(Buffer *, opcode op, size_t *addr_offset);
void bytecode_put_call(Buffer *, uint8_t locals, size_t *addr_offset);
void bytecode_put_load(Buffer *, uint8_t local);
void bytecode_put_store(Buffer *, uint8_t local);
void bytecode_put_pushint(Buffer *, uint64_t value);
void bytecode_put_pushstr(Buffer *, uint32_t offset, uint32_t len);
void bytecode_put_makecls(Buffer *, uint8_t captures, uint8_t args, size_t *addr_offset);
void bytecode_put_callcls(Buffer *, uint8_t args);
void bytecode_put_istuple(Buffer *, uint8_t len);
void bytecode_put_tupleget(Buffer *, uint8_t index);
void bytecode_put_maketuple(Buffer *, uint8_t len);
void bytecode_put_makelist(Buffer *, uint8_t len);

/*
 * Validates a compiled program and stores pointers to specific sections in `p`.
 * Does not allocate any new buffers.
 */
bool program_read(program *p, const uint8_t *bytes, size_t len);
func_meta program_func_meta(const program *p, uint32_t index);

void bytecode_debug_print(const uint8_t *bytecode, size_t bytecode_len, Buffer *output);

#endif // BYTECODE_H_
