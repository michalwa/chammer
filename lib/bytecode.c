#include "bytecode.h"

#include "utils.h"

#define BYTECODE_ADDR_PLACEHOLDER 0xFFFFFFFF

const char *opcode_name(opcode op) {
    RETURN_ENUM_NAME_V(opcode, op, EACH_OPCODE);
}

static inline uint64_t read_be_bytes(const uint8_t *bytes, size_t len) {
    uint64_t value = 0;
    for (size_t i = 0; i < len; i++) {
        value <<= 8;
        value |= bytes[i];
    }
    return value;
}

uint16_t u16be_value(u16be bytes) {
    return (uint16_t)read_be_bytes(&bytes[0], sizeof(u16be));
}

uint32_t u32be_value(u32be bytes) {
    return (uint32_t)read_be_bytes(&bytes[0], sizeof(u32be));
}

uint64_t u64be_value(u64be bytes) {
    return (uint64_t)read_be_bytes(&bytes[0], sizeof(u64be));
}

static inline void set_be_bytes(uint8_t *bytes, size_t len, uint64_t value) {
    for (size_t i = 0; i < len; i++) *bytes++ = (uint8_t)((value >> ((len - 1 - i) << 3)) & 0xFF);
}

void bytecode_set_u16be(uint8_t *bytes, uint16_t value) {
    set_be_bytes(bytes, sizeof(value), value);
}

void bytecode_set_u32be(uint8_t *bytes, uint32_t value) {
    set_be_bytes(bytes, sizeof(value), value);
}

void bytecode_set_u64be(uint8_t *bytes, uint64_t value) {
    set_be_bytes(bytes, sizeof(value), value);
}

inline void bytecode_put_u16be(Buffer *b, uint16_t value) {
    uint8_t *c = (uint8_t *)buffer_alloc(b, sizeof(value));
    bytecode_set_u16be(c, value);
}

inline void bytecode_put_u32be(Buffer *b, uint32_t value) {
    uint8_t *c = (uint8_t *)buffer_alloc(b, sizeof(value));
    bytecode_set_u32be(c, value);
}

inline void bytecode_put_u64be(Buffer *b, uint64_t value) {
    uint8_t *c = (uint8_t *)buffer_alloc(b, sizeof(value));
    bytecode_set_u64be(c, value);
}

void bytecode_put_jump(Buffer *b, opcode op, size_t *addr_offset) {
    buffer_putc(b, op);
    if (addr_offset) *addr_offset = b->len;
    bytecode_put_u32be(b, BYTECODE_ADDR_PLACEHOLDER);
}

void bytecode_put_call(Buffer *b, uint8_t locals, size_t *addr_offset) {
    buffer_putc(b, OP_CALL);
    buffer_putc(b, locals);
    if (addr_offset) *addr_offset = b->len;
    bytecode_put_u32be(b, BYTECODE_ADDR_PLACEHOLDER);
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
    bytecode_put_u64be(b, value);
}

void bytecode_put_pushstr(Buffer *b, uint32_t offset, uint32_t len) {
    buffer_putc(b, OP_PUSHSTR);
    bytecode_put_u32be(b, offset);
    bytecode_put_u32be(b, len);
}

void bytecode_put_makecls(Buffer *b, uint8_t captures, uint8_t args, size_t *addr_offset) {
    buffer_putc(b, OP_MAKECLS);
    buffer_putc(b, captures);
    buffer_putc(b, args);
    if (addr_offset) *addr_offset = b->len;
    bytecode_put_u32be(b, BYTECODE_ADDR_PLACEHOLDER);
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

bool program_read(program *p, uint8_t *bytes, size_t len) {
    if (len < 0x0E) return false;
    if (memcmp(bytes, MAGIC_HAMMER, sizeof(MAGIC_HAMMER) - 1) != 0) return false;

    uint8_t *end = bytes + len;

    p->version = (u16be *)(bytes + sizeof(MAGIC_HAMMER) - 1);
    p->string_bytes_len = (u32be *)((uint8_t *)p->version + sizeof(*p->version));
    if ((uint8_t *)p->string_bytes_len >= end) return false;

    p->string_bytes = (char *)p->string_bytes_len + sizeof(*p->string_bytes_len);

    p->bytecode = (uint8_t *)(p->string_bytes + u32be_value(*p->string_bytes_len));
    if ((uint8_t *)p->bytecode >= end) return false;

    p->bytecode_len = (size_t)(end - p->bytecode);

    return true;
}

static inline void debug_print_u8(const uint8_t *bytecode, size_t *offset, Buffer *output) {
    buffer_printf(output, " %" PRIu8, bytecode[(*offset)++]);
}

static inline void debug_print_u32(const uint8_t *bytecode, size_t *offset, Buffer *output) {
    buffer_printf(output, " %" PRIu32, u32be_value(*(u32be *)&bytecode[*offset]));
    *offset += sizeof(u32be);
}

static inline void debug_print_u32_addr(const uint8_t *bytecode, size_t *offset, Buffer *output) {
    buffer_printf(output, " %08" PRIX32, u32be_value(*(u32be *)&bytecode[*offset]));
    *offset += sizeof(u32be);
}

static inline void debug_print_u64(const uint8_t *bytecode, size_t *offset, Buffer *output) {
    buffer_printf(output, " %" PRIu64, u64be_value(*(u64be *)&bytecode[*offset]));
    *offset += sizeof(u64be);
}

void bytecode_debug_print(const uint8_t *bytecode, size_t bytecode_len, Buffer *output) {
    for (size_t offset = 0; offset < bytecode_len;) {
        uint8_t op = bytecode[offset];
        buffer_printf(output, "%08" PRIX32 "  %s", (uint32_t)offset, opcode_name(op));
        offset++;

        switch (op) {
        case OP_JUMP:
        case OP_JUMPIF:
        case OP_JUMPIFN: debug_print_u32_addr(bytecode, &offset, output); break;
        case OP_CALL:
            debug_print_u8(bytecode, &offset, output);
            debug_print_u32_addr(bytecode, &offset, output);
            break;
        case OP_LOAD:
        case OP_STORE:
        case OP_CALLCLS:
        case OP_ISTUPLE:
        case OP_TUPLEGET:
        case OP_MAKETUPLE:
        case OP_MAKELIST: debug_print_u8(bytecode, &offset, output); break;
        case OP_PUSHINT: debug_print_u64(bytecode, &offset, output); break;
        case OP_PUSHSTR:
            debug_print_u32(bytecode, &offset, output);
            debug_print_u32(bytecode, &offset, output);
            break;
        case OP_MAKECLS:
            debug_print_u8(bytecode, &offset, output);
            debug_print_u8(bytecode, &offset, output);
            debug_print_u32_addr(bytecode, &offset, output);
            break;
        }

        buffer_putc(output, '\n');
    }
}
