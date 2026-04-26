#include "bytecode.h"

#include "bytes.h"
#include "utils.h"

#define BYTECODE_ADDR_PLACEHOLDER 0xFFFFFFFF

const char *opcode_name(opcode op) {
    RETURN_ENUM_NAME_V(opcode, op, EACH_OPCODE);
}

void bytecode_put_jump(Buffer *b, opcode op, size_t *addr_offset) {
    debug_assert(op == OP_JUMP || op == OP_JUMPIF || op == OP_JUMPIFN);

    buffer_putc(b, op);
    if (addr_offset) *addr_offset = b->len;
    buffer_put_u32be(b, BYTECODE_ADDR_PLACEHOLDER);
}

void bytecode_put_call(Buffer *b, uint32_t fnindex) {
    buffer_putc(b, OP_CALL);
    buffer_put_u32be(b, fnindex);
}

void bytecode_put_load(Buffer *b, uint8_t local) {
    buffer_putc(b, OP_LOAD);
    buffer_putc(b, local);
}

void bytecode_put_store(Buffer *b, uint8_t local) {
    buffer_putc(b, OP_STORE);
    buffer_putc(b, local);
}

void bytecode_put_pushint(Buffer *b, uint64_t value) {
    buffer_putc(b, OP_PUSHINT);
    buffer_put_u64be(b, value);
}

void bytecode_put_pushstr(Buffer *b, uint32_t offset, uint32_t len) {
    buffer_putc(b, OP_PUSHSTR);
    buffer_put_u32be(b, offset);
    buffer_put_u32be(b, len);
}

void bytecode_put_makecls(Buffer *b, uint32_t fnindex, uint8_t captures) {
    buffer_putc(b, OP_MAKECLS);
    buffer_put_u32be(b, fnindex);
    buffer_putc(b, captures);
}

void bytecode_put_callcls(Buffer *b, uint8_t args) {
    buffer_putc(b, OP_CALLCLS);
    buffer_putc(b, args);
}

void bytecode_put_istuple(Buffer *b, uint8_t len) {
    buffer_putc(b, OP_ISTUPLE);
    buffer_putc(b, len);
}

void bytecode_put_tupleget(Buffer *b, uint8_t index) {
    buffer_putc(b, OP_TUPLEGET);
    buffer_putc(b, index);
}

void bytecode_put_maketuple(Buffer *b, uint8_t len) {
    buffer_putc(b, OP_MAKETUPLE);
    buffer_putc(b, len);
}

void bytecode_put_makelist(Buffer *b, uint8_t len) {
    buffer_putc(b, OP_MAKELIST);
    buffer_putc(b, len);
}

bool program_read(program *p, const uint8_t *bytes, size_t len) {
    if (len < sizeof(MAGIC_HAMMER) + 5) return false;
    if (memcmp(bytes, MAGIC_HAMMER, sizeof(MAGIC_HAMMER) - 1) != 0) return false;

    const uint8_t *cursor = bytes + sizeof(MAGIC_HAMMER) - 1;
    const uint8_t *end = bytes + len;

    p->version = read_u16be(&cursor);
    p->string_bytes_len = read_u32be(&cursor);
    p->string_bytes = (const char *)cursor;

    cursor += p->string_bytes_len;
    if ((intptr_t)end - (intptr_t)cursor < 4) return false;

    p->funcs_len = read_u32be(&cursor);
    p->funcs = (func_meta_bytes *)cursor;

    cursor += sizeof(func_meta_bytes) * p->funcs_len;
    if ((intptr_t)end - (intptr_t)cursor < 4) return false;

    p->bytecode = cursor;
    p->bytecode_len = (size_t)(end - p->bytecode);

    return true;
}

func_meta program_func_meta(const program *p, uint32_t index) {
    func_meta fn;
    const uint8_t *cursor = (const uint8_t *)(p->funcs + index);

    fn.addr = read_u32be(&cursor);
    fn.locals = *cursor++;
    fn.args = *cursor++;

    return fn;
}

void bytecode_put_func_meta(Buffer *b, func_meta fn) {
    buffer_put_u32be(b, fn.addr);
    buffer_putc(b, fn.locals);
    buffer_putc(b, fn.args);
}

static inline void debug_print_u8(const uint8_t **b, Buffer *output) {
    buffer_printf(output, " %" PRIu8, *(*b)++);
}

static inline void debug_print_u32(const uint8_t **b, Buffer *output) {
    buffer_printf(output, " %" PRIu32, read_u32be(b));
}

static inline void debug_print_u32_addr(const uint8_t **b, Buffer *output) {
    buffer_printf(output, " %08" PRIX32, read_u32be(b));
}

static inline void debug_print_u64(const uint8_t **b, Buffer *output) {
    buffer_printf(output, " %" PRIu64, read_u64be(b));
}

void bytecode_debug_print(const uint8_t *bytecode, size_t bytecode_len, Buffer *output) {
    const uint8_t *cursor = bytecode;
    const uint8_t *end = bytecode + bytecode_len;

    while (cursor < end) {
        uint32_t offset = (uint32_t)(cursor - bytecode);
        uint8_t op = *cursor++;

        buffer_printf(output, "%08" PRIX32 "  %s", (uint32_t)offset, opcode_name(op));

        switch (op) {
        case OP_JUMP:
        case OP_JUMPIF:
        case OP_JUMPIFN: debug_print_u32_addr(&cursor, output); break;
        case OP_CALL:
            debug_print_u32(&cursor, output);
            break;
        case OP_LOAD:
        case OP_STORE:
        case OP_CALLCLS:
        case OP_ISTUPLE:
        case OP_TUPLEGET:
        case OP_MAKETUPLE:
        case OP_MAKELIST: debug_print_u8(&cursor, output); break;
        case OP_PUSHINT: debug_print_u64(&cursor, output); break;
        case OP_PUSHSTR:
            debug_print_u32(&cursor, output);
            debug_print_u32(&cursor, output);
            break;
        case OP_MAKECLS:
            debug_print_u32(&cursor, output);
            debug_print_u8(&cursor, output);
            break;
        }

        buffer_putc(output, '\n');
    }
}
