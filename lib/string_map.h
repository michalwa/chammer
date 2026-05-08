#ifndef HAMMER_STRING_MAP_H_
#define HAMMER_STRING_MAP_H_

#include <inttypes.h>
#include <stdbool.h>

#include "arena.h"
#include "buffer.h"
#include "string.h"

#define string_map_init(m, t)            string_map_init_(m, sizeof(t))
#define string_map_init_buckets(m, t, b) string_map_init_buckets_(m, sizeof(t), b)

typedef struct string_map_entry string_map_entry;
struct string_map_entry {
    uint64_t          hash;
    string            key;     // points to `StringMap.key_buffer`
    string_map_entry *next;    // points to `StringMap.entries`
    const uint8_t     value[]; // size is `StringMap.value_size`
};

/*
 * Hash map with string keys
 */
typedef struct {
    size_t             value_size;
    size_t             num_buckets;
    string_map_entry **bucket_heads; // points to `entries`
    Buffer             key_buffer;
    Arena              entries;
} StringMap;

void string_map_init_(StringMap *, size_t value_size);
void string_map_init_buckets_(StringMap *, size_t value_size, size_t num_buckets);
void string_map_free(StringMap *);
/*
 * Inserts a value at the given key. The value is copied from the pointer, so
 * it is not needed to keep the pointer valid after this function returns.
 * Returns `true` if a previous value was present and overwritten, and stores
 * the previous value in `old_value` if not `NULL`.
 */
bool string_map_put(StringMap *, string key, const void *value, void *old_value);
/*
 * Finds the value at the given key and stores it in `value` if not `NULL`.
 * Returns `true` if found or `false` otherwise.
 */
bool string_map_get(const StringMap *, string key, void *value);
/*
 * Finds the entry with the given key and returns a pointer to it. Returns
 * `NULL` if not found.
 */
const string_map_entry *string_map_get_entry(const StringMap *, string key);

#endif // HAMEMR_STRING_MAP_H_
