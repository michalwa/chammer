#include "string_pool.h"

#include "string_map.h"

void string_pool_init(StringPool *sp) {
    string_map_init_(&sp->map, 0);
}

void string_pool_free(StringPool *sp) {
    string_map_free(&sp->map);
}

inline symbol string_pool_intern(StringPool *sp, string str) {
    string_map_put(&sp->map, str, NULL, NULL);
    return (symbol)string_map_get_entry(&sp->map, str);
}

inline string_pool_entry string_pool_get_entry(StringPool *sp, symbol sym) {
    return (string_pool_entry){
        .offset = sym->key.data - sp->map.key_buffer.data,
        .len = sym->key.len,
    };
}

inline size_t string_pool_size(StringPool *sp) {
    return sp->map.key_buffer.len;
}

inline string string_pool_contents(StringPool *sp) {
    return buffer_string(&sp->map.key_buffer);
}

inline string symbol_string(symbol sym) {
    return (string){ .data = sym->key.data, .len = sym->key.len };
}
