#ifndef STRING_H_
#define STRING_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char *data;
    size_t      len;
} string;

#define F_STRING "%.*s"
#define FA_STRING(str) (int)(str).len, (str).data

#define STRING(str) (string){ .data = str, .len = sizeof(str) - 1 }

bool   string_eq(string a, string b);
string string_from_cstr(const char *str);

#endif // STRING_H_
