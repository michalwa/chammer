#ifndef STRING_H_
#define STRING_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    char  *data;
    size_t len;
} string;

#define F_STRING          "%.*s"
#define FA_STRING(string) (int)(string).len, (string).data

bool string_eq(string a, string b);

#endif // STRING_H_
