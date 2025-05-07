#ifndef _STRINGS_H
#define _STRINGS_H

#include <inttypes.h>
#include <stdlib.h>

typedef struct {
    char  *buffer;
    size_t capacity;
    size_t cursor;
} Strings;

typedef struct {
    char  *data;
    size_t len;
} string;

#define F_STR "%.*s"
#define FA_STR(str) (int)(str).len, (str).data

void   strings_init(Strings *s);
string strings_alloc(Strings *s, size_t len);
void   strings_free(Strings *s);

#endif // _STRINGS_H
