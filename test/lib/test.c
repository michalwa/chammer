#include "test.h"

#include <ctype.h>

void buffer_print_c_string_literal(Buffer *b, const char *str) {
    buffer_putc(b, '"');

    for (const char *c = str; *c; c++) {
        switch (*c) {
        case '\0': buffer_printf(b, "\\0"); break;
        case '\a': buffer_printf(b, "\\a"); break;
        case '\b': buffer_printf(b, "\\b"); break;
        case '\f': buffer_printf(b, "\\f"); break;
        case '\n': buffer_printf(b, "\\n"); break;
        case '\r': buffer_printf(b, "\\r"); break;
        case '\t': buffer_printf(b, "\\t"); break;
        case '\v': buffer_printf(b, "\\v"); break;
        case '\\': buffer_printf(b, "\\\\"); break;
        case '\"': buffer_printf(b, "\\\""); break;
        default:
            if (isprint(*c))
                buffer_printf(b, "%c", *c);
            else
                buffer_printf(b, "\\%03o", *c);
        }
    }

    buffer_putc(b, '"');
}
