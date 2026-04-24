#include "bytecode.h"

#include "utils.h"

#define BYTECODE_ADDR_PLACEHOLDER 0xFFFFFFFF

const char *opcode_name(opcode op) {
    RETURN_ENUM_NAME_V(opcode, op, EACH_OPCODE);
}

#define read_be_bytes(b, t)                      \
    do {                                         \
        t v = 0;                                 \
        for (size_t i = 0; i < sizeof(t); i++) { \
            v <<= 8;                             \
            v |= b[i];                           \
        }                                        \
        return v;                                \
    } while (0)

inline uint16_t u16be_value(u16be bytes) {
    read_be_bytes(bytes, uint16_t);
}

inline uint32_t u32be_value(u32be bytes) {
    read_be_bytes(bytes, uint32_t);
}

inline uint64_t u64be_value(u64be bytes) {
    read_be_bytes(bytes, uint64_t);
}

#undef read_be_bytes

#define set_be_bytes(c, v)                                                                      \
    do {                                                                                        \
        for (intptr_t i = sizeof(v) - 1; i >= 0; i--) *c++ = (uint8_t)((v >> (i << 3)) & 0xFF); \
    } while (0)

inline void bytecode_set_u16be(char *c, uint16_t v) {
    set_be_bytes(c, v);
}

inline void bytecode_set_u32be(char *c, uint32_t v) {
    set_be_bytes(c, v);
}

inline void bytecode_set_u64be(char *c, uint64_t v) {
    set_be_bytes(c, v);
}

#undef set_be_bytes

inline void bytecode_put_u16be(Buffer *b, uint16_t v) {
    char *c = buffer_alloc(b, sizeof(v));
    bytecode_set_u16be(c, v);
}

inline void bytecode_put_u32be(Buffer *b, uint32_t v) {
    char *c = buffer_alloc(b, sizeof(v));
    bytecode_set_u32be(c, v);
}

inline void bytecode_put_u64be(Buffer *b, uint64_t v) {
    char *c = buffer_alloc(b, sizeof(v));
    bytecode_set_u64be(c, v);
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

static void debug_print_u8(const uint8_t *bytecode, size_t *offset, Buffer *output) {
    buffer_printf(output, " %" PRIu8, bytecode[(*offset)++]);
}

static void debug_print_u32(const uint8_t *bytecode, size_t *offset, Buffer *output) {
    buffer_printf(output, " %" PRIu32, u32be_value(*(u32be *)&bytecode[*offset]));
    *offset += sizeof(u32be);
}

static void debug_print_u32_addr(const uint8_t *bytecode, size_t *offset, Buffer *output) {
    buffer_printf(output, " %08" PRIX32, u32be_value(*(u32be *)&bytecode[*offset]));
    *offset += sizeof(u32be);
}

static void debug_print_u64(const uint8_t *bytecode, size_t *offset, Buffer *output) {
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
        case OP_TUPLEGET: debug_print_u8(bytecode, &offset, output); break;
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
