#include "bytecode.h"

#include "bytes.h"
#include "utils.h"

#define BYTECODE_ADDR_PLACEHOLDER (int16_t)0xFFFF

const char *opcode_name(opcode op) {
    RETURN_ENUM_NAME_V(opcode, op, EACH_OPCODE);
}

void bytecode_put_jump(Buffer *b, opcode op, size_t *addr_offset) {
    debug_assert(op == OP_JUMP || op == OP_JUMPIF || op == OP_JUMPIFN);

    buffer_putc(b, op);
    if (addr_offset) *addr_offset = b->len;
    buffer_put_i16be(b, BYTECODE_ADDR_PLACEHOLDER);
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

void bytecode_put_pushint(Buffer *b, int64_t value) {
    buffer_putc(b, OP_PUSHINT);
    buffer_put_i64be(b, value);
}

void bytecode_put_pushstr(Buffer *b, uint32_t offset, uint32_t len) {
    buffer_putc(b, OP_PUSHSTR);
    buffer_put_u32be(b, offset);
    buffer_put_u32be(b, len);
}

void bytecode_put_makecls(Buffer *b, uint32_t fnindex) {
    buffer_putc(b, OP_MAKECLS);
    buffer_put_u32be(b, fnindex);
}

void bytecode_put_callval(Buffer *b, uint8_t args) {
    buffer_putc(b, OP_CALLVAL);
    buffer_putc(b, args);
}

void bytecode_put_istuple(Buffer *b, uint16_t len) {
    buffer_putc(b, OP_ISTUPLE);
    buffer_put_u16be(b, len);
}

void bytecode_put_tupleget(Buffer *b, uint16_t index) {
    buffer_putc(b, OP_TUPLEGET);
    buffer_put_u16be(b, index);
}

void bytecode_put_maketuple(Buffer *b, uint16_t len) {
    buffer_putc(b, OP_MAKETUPLE);
    buffer_put_u16be(b, len);
}

void bytecode_put_makelist(Buffer *b, uint16_t len) {
    buffer_putc(b, OP_MAKELIST);
    buffer_put_u16be(b, len);
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

void bytecode_put_func_meta(Buffer *b, func_meta *fn) {
    func_meta_bytes *bytes = (func_meta_bytes *)buffer_alloc(b, sizeof(func_meta_bytes));

    bytes->locals = fn->locals;
    bytes->args = fn->args;
    bytes->captures = fn->captures;
    bytes->type = (uint8_t)fn->type;
    bytes->name_len = fn->name_len;

    uint8_t *cursor;
    cursor = (uint8_t *)&bytes->addr;
    write_u32be(&cursor, fn->addr);
    cursor = (uint8_t *)&bytes->name_offset;
    write_u32be(&cursor, fn->name_offset);
}

void program_func_meta(const program *p, uint32_t index, func_meta *fn) {
    func_meta_bytes *bytes = &p->funcs[index];

    fn->locals = bytes->locals;
    fn->args = bytes->args;
    fn->captures = bytes->captures;
    fn->type = bytes->type;
    fn->name_len = bytes->name_len;

    const uint8_t *cursor;
    cursor = (const uint8_t *)&bytes->addr;
    fn->addr = read_u32be(&cursor);
    cursor = (const uint8_t *)&bytes->name_offset;
    fn->name_offset = read_u32be(&cursor);
}

string program_func_name(const program *prog, func_meta *fn) {
    switch (fn->type) {
    case FN_NAMED:
        return (string){ .data = prog->string_bytes + fn->name_offset, .len = fn->name_len };
    case FN_BLOCK: return STRING("<block>");
    case FN_LAMBDA: return STRING("<lambda>");
    case FN_CASE: return STRING("<case>");
    case FN_GLOBAL: return STRING("<global>");
    }
}

void program_debug_print(const program *prog, Buffer *output) {
    const uint8_t *cursor = prog->bytecode;
    const uint8_t *end = prog->bytecode + prog->bytecode_len;

    uint32_t  fn_index, str_offset, str_len;
    int16_t   jump;
    func_meta fn;
    string    fn_name;

    while (cursor < end) {
        uint32_t offset = (uint32_t)(cursor - prog->bytecode);
        uint8_t  op = *cursor++;

        for (size_t i = 0; i < prog->funcs_len; i++) {
            program_func_meta(prog, i, &fn);
            if (fn.addr == offset) {
                fn_name = program_func_name(prog, &fn);
                buffer_printf(
                    output, F_STRING "  %" PRIu8 " locals, %" PRIu8 " captures, %" PRIu8 " args\n",
                    FA_STRING(fn_name), fn.locals, fn.captures, fn.args
                );
                break;
            }
        }

        buffer_printf(output, "  %08" PRIX32 "  %s", (uint32_t)offset, opcode_name(op));

        switch (op) {
        case OP_JUMP:
        case OP_JUMPIF:
        case OP_JUMPIFN:
            jump = read_i16be(&cursor);
            buffer_printf(output, " %+" PRIi16 " (%08" PRIX32 ")", jump, offset + jump);
            break;
        case OP_CALL:
        case OP_MAKECLS:
            fn_index = read_u32be(&cursor);
            program_func_meta(prog, fn_index, &fn);
            fn_name = program_func_name(prog, &fn);
            buffer_printf(
                output, " %" PRIu32 " (" F_STRING " @ %08" PRIX32 ")", fn_index, FA_STRING(fn_name),
                fn.addr
            );
            break;
        case OP_LOAD:
        case OP_STORE:
        case OP_CALLVAL: buffer_printf(output, " %" PRIu8, *cursor++); break;
        case OP_ISTUPLE:
        case OP_TUPLEGET:
        case OP_MAKETUPLE:
        case OP_MAKELIST: buffer_printf(output, " %" PRIu16, read_u16be(&cursor)); break;
        case OP_PUSHINT: buffer_printf(output, " %" PRIi64, read_i64be(&cursor)); break;
        case OP_PUSHSTR:
            str_offset = read_u32be(&cursor);
            str_len = read_u32be(&cursor);
            buffer_printf(output, " %" PRIu32 " %" PRIu32 " (", str_offset, str_len);
            buffer_print_string_literal(
                output, (string){ .data = prog->string_bytes + str_offset, .len = str_len }
            );
            buffer_putc(output, ')');
            break;
        }

        buffer_putc(output, '\n');
    }
}
