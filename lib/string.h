#ifndef STRING_H_
#define STRING_H_

#include <inttypes.h>

typedef struct {
    char  *data;
    size_t len;
} string;

#define F_STRING          "%.*s"
#define FA_STRING(string) (int)(string).len, (string).data

#endif // STRING_H_
