#ifndef STRING_H_
#define STRING_H_

#include <inttypes.h>
#include <stdbool.h>

typedef struct {
    const char *data;
    size_t      len;
} string;

#define STRING(str) (string){ .data = str, .len = sizeof(str) - 1 }

bool string_eq(string a, string b);

#endif // STRING_H_
