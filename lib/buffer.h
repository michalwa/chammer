#ifndef BUFFER_H_
#define BUFFER_H_

#include <inttypes.h>
#include <stdio.h>

#include "string.h"

typedef struct Buffer {
    /*
     * Ensured to point to a valid null-terminated string
     */
    char *data;
    /*
     * Does not include the null terminator
     */
    size_t len;
    size_t capacity;
} Buffer;

#define F_BUFFER          "%.*s"
#define FA_BUFFER(buffer) (int)(buffer).len, (buffer).data

void  buffer_init(Buffer *);
void  buffer_init_capacity(Buffer *, size_t);
void  buffer_putc(Buffer *, char);
void  buffer_printf(Buffer *, const char *format, ...);
char *buffer_alloc(Buffer *, size_t len);
void  buffer_free(Buffer *);

void fread_to_buffer(FILE *, Buffer *);

#endif // BUFFER_H_
