#include "bytecode.h"
#include "utils.h"

#define read_be_bytes(b, t) \
    do { \
        t v = 0; \
        for (size_t i = 0; i < sizeof(t); i++) { \
            v <<= 8; \
            v |= b[i]; \
        } \
        return v; \
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

#define write_be_bytes(b, v) \
    do { \
        char *c = buffer_alloc(b, sizeof(v)); \
        for (intptr_t i = sizeof(v) - 1; i >= 0; i--) \
            *c++ = (uint8_t)((v >> (i << 3)) & 0xFF); \
    } while (0)

inline void buffer_write_u16be(Buffer *b, uint16_t v) {
    write_be_bytes(b, v);
}

inline void buffer_write_u32be(Buffer *b, uint32_t v) {
    write_be_bytes(b, v);
}

inline void buffer_write_u64be(Buffer *b, uint64_t v) {
    write_be_bytes(b, v);
}

#undef write_be_bytes

inline size_t opcode_data_size(opcode op) {
#define OPCODE_CASE(name, byte, data_size) \
    case name: return data_size;

    switch (op) { EACH_OPCODE(OPCODE_CASE) }
#undef OPCODE_CASE

    panic("unknown opcode: %02X", op);
}

bool program_read(program *p, uint8_t *bytes, size_t len) {
    if (len < 0x0E) return false;
    if (memcmp(bytes, MAGIC_HAMMER, sizeof(MAGIC_HAMMER) - 1) != 0) return false;

    uint8_t *end = bytes + len;

    p->version = (u16be *)(bytes + sizeof(MAGIC_HAMMER) - 1);
    p->trace_table_len = (u16be *)((uint8_t *)p->version + sizeof(*p->version));
    p->trace_table = (trace_info *)((uint8_t *)p->trace_table_len + sizeof(*p->trace_table_len));

    p->string_bytes_len = (u32be *)(p->trace_table + u16be_value(*p->trace_table_len));
    if ((uint8_t *)p->string_bytes_len >= end) return false;

    p->string_bytes = (char *)p->string_bytes_len + sizeof(*p->string_bytes_len);

    p->bytecode = (uint8_t *)(p->string_bytes + u32be_value(*p->string_bytes_len));
    if ((uint8_t *)p->bytecode >= end) return false;

    p->bytecode_len = (size_t)(end - p->bytecode);

    return true;
}
