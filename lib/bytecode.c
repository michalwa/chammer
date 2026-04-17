#include "bytecode.h"

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

bool program_read(program *p, uint8_t *bytes, size_t len) {
    if (len < 0x0E) return false;
    if (memcmp(bytes, MAGIC_HAMMER, sizeof(MAGIC_HAMMER) - 1) != 0) return false;

    uint8_t *end = bytes + len;

    p->hammer_version = (u16be *)(bytes + sizeof(MAGIC_HAMMER) - 1);
    p->trace_table_len = (u16be *)((uint8_t *)p->hammer_version + sizeof(*p->hammer_version));
    p->trace_table = (trace_info *)((uint8_t *)p->trace_table_len + sizeof(*p->trace_table_len));

    p->string_bytes_len = (u32be *)(p->trace_table + u16be_value(*p->trace_table_len));
    if ((uint8_t *)p->string_bytes_len >= end) return false;

    p->string_bytes = (char *)p->string_bytes_len + sizeof(*p->string_bytes_len);

    p->bytecode = (uint8_t *)(p->string_bytes + u32be_value(*p->string_bytes_len));
    if ((uint8_t *)p->bytecode >= end) return false;

    p->bytecode_len = (size_t)(end - p->bytecode);

    return true;
}
