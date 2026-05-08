#include "string_map.h"

#include <stdlib.h>
#include <string.h>

#include "arena.h"

#define STRING_MAP_DEFAULT_NUM_HEADS 0x400

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
#define FNV_OFFSET_BASIS 0xCBF29CE484222325
#define FNV_PRIME        0x100000001B3

static uint64_t string_hash(string str) {
    // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash
    uint64_t hash = FNV_OFFSET_BASIS;

    for (size_t i = 0; i < str.len; i++) hash = (hash * FNV_PRIME) ^ str.data[i];

    return hash;
}

inline void string_map_init_(StringMap *m, size_t value_size) {
    string_map_init_buckets_(m, value_size, STRING_MAP_DEFAULT_NUM_HEADS);
}

void string_map_init_buckets_(StringMap *m, size_t value_size, size_t num_buckets) {
    m->value_size = value_size;
    m->num_buckets = num_buckets;
    m->bucket_heads = calloc(num_buckets, sizeof(*m->bucket_heads));
    buffer_init(&m->key_buffer);

    size_t alignment = sizeof(string_map_entry);
    size_t padding = (alignment - (value_size % alignment)) % alignment;

    arena_init_(&m->entries, sizeof(string_map_entry) + value_size + padding);
}

void string_map_free(StringMap *m) {
    free(m->bucket_heads);
    buffer_free(&m->key_buffer);
    arena_free(&m->entries);
}

static string_map_entry **string_map_get_entry_(
    const StringMap *m, string key, string_map_entry **prev, uint64_t *out_hash
) {
    uint64_t hash = string_hash(key);
    if (out_hash) *out_hash = hash;

    string_map_entry **head = &m->bucket_heads[hash % m->num_buckets];
    string_map_entry **entry = head;

    for (entry = head; *entry; entry = &(*entry)->next) {
        if ((*entry)->hash == hash && string_eq((*entry)->key, key)) return entry;
        if (prev) *prev = *entry;
    }

    return entry;
}

bool string_map_put(StringMap *m, string key, const void *value, void *old_value) {
    uint64_t           hash;
    string_map_entry  *prev = NULL;
    string_map_entry **entry = string_map_get_entry_(m, key, &prev, &hash);

    if (*entry) {
        if (old_value) memcpy(old_value, (*entry)->value, m->value_size);
        memcpy((void *)(*entry)->value, value, m->value_size);
        return true;
    }

    *entry = arena_push_zeroed(&m->entries);
    if (prev) prev->next = *entry;
    (*entry)->hash = hash;
    (*entry)->key = buffer_puts(&m->key_buffer, key);
    memcpy((void *)(*entry)->value, value, m->value_size);

    return false;
}

bool string_map_get(const StringMap *m, string key, void *value) {
    const string_map_entry *entry = string_map_get_entry(m, key);
    if (!entry) return false;
    if (value) memcpy(value, entry->value, m->value_size);
    return true;
}

const string_map_entry *string_map_get_entry(const StringMap *m, string key) {
    return *string_map_get_entry_(m, key, NULL, NULL);
}

string_map_iter string_map_iter_begin(const StringMap *m) {
    return (string_map_iter){
        .map = m,
        .bucket = 0,
        .entry = NULL,
    };
}

bool string_map_iter_next(string_map_iter *iter) {
    if (iter->entry && iter->entry->next) {
        iter->entry = iter->entry->next;
        return true;
    }

    iter->entry = NULL;
    while (!iter->entry && iter->bucket < iter->map->num_buckets)
        iter->entry = iter->map->bucket_heads[iter->bucket++];

    return !!iter->entry;
}
