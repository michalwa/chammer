#ifndef BUFFER_H_
#define BUFFER_H_

#include <inttypes.h>
#include <stdio.h>

#include "string.h"

/*
 * A growable container of bytes
 */
typedef struct {
    /*
     * Ensured to point to a valid null-terminated string
     */
    char  *data;
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
void  buffer_free(Buffer *);
void  buffer_putc(Buffer *, char);
void  buffer_printf(Buffer *, const char *format, ...);
/*
 * Allocates `len` additional bytes in the buffer and returns a pointer to the allocated block
 */
char *buffer_alloc(Buffer *, size_t len);
void  buffer_clear(Buffer *);
/*
 * Appends the contents of the given file to the buffer
 */
void  buffer_read_file(Buffer *, FILE *);

#endif // BUFFER_H_
