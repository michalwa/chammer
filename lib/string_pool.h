#ifndef HAMMER_STRING_POOL_H_
#define HAMMER_STRING_POOL_H_

#include "string_map.h"

/*
 * Minimal wrapper around `StringMap` for interning strings
 */
typedef struct {
    StringMap map;
} StringPool;

typedef struct {
    size_t offset;
    size_t len;
} string_pool_entry;

typedef const string_map_entry *symbol;

void              string_pool_init(StringPool *);
void              string_pool_free(StringPool *);
symbol            string_pool_intern(StringPool *, string);
string_pool_entry string_pool_get_entry(StringPool *, symbol);
size_t            string_pool_size(StringPool *);
string            string_pool_contents(StringPool *);
string            symbol_string(symbol);

#endif // HAMMER_STRING_POOL_H_
