#include "bytecode.h"
#include "utils.h"

inline uint16_t u16be_value(u16be bytes) {
    return ((uint16_t)bytes[0] << (1 << 3))
        | (uint16_t)bytes[1];
}

inline uint32_t u32be_value(u32be bytes) {
    return ((uint32_t)bytes[0] << (3 << 3))
        | ((uint32_t)bytes[1] << (2 << 3))
        | ((uint32_t)bytes[2] << (1 << 3))
        | (uint32_t)bytes[3];
}

inline size_t opcode_data_size(opcode op) {
#define OPCODE_CASE(name, byte, data_size) \
    case name: return data_size;

    switch (op) { EACH_OPCODE(OPCODE_CASE) }
#undef OPCODE_CASE

    panic("unknown opcode: %02X", op);
}

inline void buffer_write_u8be(Buffer *b, uint8_t v) {
    char *out = buffer_alloc(b, 1);
    *out++ = v;
}

inline void buffer_write_u16be(Buffer *b, uint16_t v) {
    char *out = buffer_alloc(b, 2);
    *out++ = (uint8_t)((v >> (1 << 3)) & 0xFF);
    *out++ = (uint8_t)(v & 0xFF);
}

inline void buffer_write_u32be(Buffer *b, uint32_t v) {
    char *out = buffer_alloc(b, 4);
    *out++ = (uint8_t)((v >> (3 << 3)) & 0xFF);
    *out++ = (uint8_t)((v >> (2 << 3)) & 0xFF);
    *out++ = (uint8_t)((v >> (1 << 3)) & 0xFF);
    *out++ = (uint8_t)(v & 0xFF);
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
