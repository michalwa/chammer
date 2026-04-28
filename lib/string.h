#ifndef HAMMER_STRING_H_
#define HAMMER_STRING_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char *data;
    size_t      len;
} string;

#define F_STRING       "%.*s"
#define FA_STRING(str) (int)(str).len, (str).data

#define STRING(str) (string){ .data = str, .len = sizeof(str) - 1 }

bool   string_eq(string a, string b);
string string_from_cstr(const char *str);

void next_line(const char **line, size_t *len);

#endif // HAMMER_STRING_H_
