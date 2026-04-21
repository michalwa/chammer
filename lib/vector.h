#ifndef VECTOR_H_
#define VECTOR_H_

#include <stddef.h>

/*
 * A growable homogenous container which can move in memory
 */
typedef struct {
    size_t item_size;
    void *data;
    size_t len;
    size_t capacity_items;
} Vector;

/*
 * Usage: `for (EACH_IN_VECTOR(v, t, i)) { ... }`
 */
#define EACH_IN_VECTOR(v, t, i) \
    t *i = (t *)(v).data; \
    (size_t)(i - (t *)(v).data) < (v).len; \
    i++

#define vector_init(v, t) vector_init_(v, sizeof(t))
#define vector_init_capacity(v, t, c) vector_init_capacity_(v, sizeof(t), c)

void vector_init_(Vector *, size_t item_size);
void vector_init_capacity_(Vector *, size_t item_size, size_t capacity_items);
void vector_free(Vector *);
void *vector_push(Vector *);
void *vector_push_zeroed(Vector *);
void vector_pop(Vector *);
void *vector_last(Vector *);
void vector_clear(Vector *);
void *vector_get(Vector *, size_t index);

#endif // VECTOR_H_
