#ifndef _STRING_H
#define _STRING_H

#include <inttypes.h>

typedef struct {
    char  *data;
    size_t len;
} string;

#define F_STRING          "%.*s"
#define FA_STRING(string) (int)(string).len, (string).data

#endif // _STRING_H
