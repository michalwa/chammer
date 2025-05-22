#include "string.h"

#include <string.h>

inline bool string_eq(string a, string b) {
    return a.len == b.len && strncmp(a.data, b.data, a.len) == 0;
}
