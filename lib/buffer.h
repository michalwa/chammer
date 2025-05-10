#ifndef BUFFER_H_
#define BUFFER_H_

#include <inttypes.h>

#include "string.h"

typedef struct {
    char  *data;
    size_t len;
    size_t capacity;
} Buffer;

#define F_BUFFER          "%.*s"
#define FA_BUFFER(buffer) (int)(buffer).len, (buffer).data

void buffer_init(Buffer *);
void buffer_init_capacity(Buffer *, size_t);
void buffer_printf(Buffer *, const char *format, ...);
void buffer_free(Buffer *);

#endif // BUFFER_H_
