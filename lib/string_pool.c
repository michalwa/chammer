#include "string_pool.h"

#include <inttypes.h>
#include <string.h>

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
#define FNV_OFFSET_BASIS 0xCBF29CE484222325
#define FNV_PRIME 0x100000001B3

static uint64_t string_hash(string str) {
    // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash
    uint64_t hash = FNV_OFFSET_BASIS;

    for (size_t i = 0; i < str.len; i++)
        hash = (hash * FNV_PRIME) ^ str.data[i];

    return hash;
}

static inline string entry_string(StringPool *sp, string_pool_entry e) {
    return (string){ .data = sp->buffer.data + e.offset, .len = e.len };
}

void string_pool_init(StringPool *sp) {
    buffer_init(&sp->buffer);
    vector_init(&sp->entries, string_pool_entry);
}

void string_pool_free(StringPool *sp) {
    buffer_free(&sp->buffer);
    vector_free(&sp->entries);
}

symbol string_pool_intern(StringPool *sp, string str) {
    uint64_t hash = string_hash(str);

    for (size_t i = 0; i < sp->entries.len; i++) {
        string_pool_entry *e = (string_pool_entry *)vector_get(&sp->entries, i);

        if (hash == e->hash && string_eq(str, entry_string(sp, *e))) return i;
    }

    symbol             id = sp->entries.len;
    string_pool_entry *new_entry = (string_pool_entry *)vector_push(&sp->entries);
    new_entry->len = str.len;
    new_entry->hash = hash;

    if (sp->buffer.len >= str.len) {
        for (size_t offset = 0; offset < sp->buffer.len - str.len; offset++) {
            if (memcmp(sp->buffer.data + offset, str.data, str.len) == 0) {
                new_entry->offset = offset;
                return id;
            }
        }
    }

    new_entry->offset = sp->buffer.len;
    buffer_puts(&sp->buffer, str);
    return id;
}

string string_pool_get(StringPool *sp, symbol id) {
    string_pool_entry *e = (string_pool_entry *)vector_get(&sp->entries, id);
    return entry_string(sp, *e);
}
