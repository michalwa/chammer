#ifndef HAMMER_STRING_POOL_H_
#define HAMMER_STRING_POOL_H_

#include "buffer.h"
#include "string.h"
#include "vector.h"

typedef struct {
    Buffer buffer;
    Vector entries;
} StringPool;

typedef struct {
    size_t   offset;
    size_t   len;
    uint64_t hash;
} string_pool_entry;

typedef size_t symbol;

void   string_pool_init(StringPool *);
void   string_pool_free(StringPool *);
symbol string_pool_intern(StringPool *, string);
string string_pool_get(StringPool *, symbol);

#endif // HAMMER_STRING_POOL_H_
