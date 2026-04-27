#include "string.h"

#include <string.h>

bool string_eq(string a, string b) {
    return a.len == b.len && strncmp(a.data, b.data, a.len) == 0;
}

inline string string_from_cstr(const char *str) {
    return (string){ .data = str, .len = strlen(str) };
}

void next_line(const char **line, size_t *len) {
    *line += *len;

    if (**line == '\r') (*line)++;
    if (**line == '\n') (*line)++;

    *len = strcspn(*line, "\r\n");
}
