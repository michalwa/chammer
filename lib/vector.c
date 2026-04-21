#include "vector.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_DEFAULT_MAX_SIZE 0x400

void vector_init_(Vector *v, size_t item_size) {
    vector_init_capacity_(v, item_size, VECTOR_DEFAULT_MAX_SIZE / item_size);
}

void vector_init_capacity_(Vector *v, size_t item_size, size_t capacity_items) {
    v->item_size = item_size;
    v->capacity_items = capacity_items;
    v->len = 0;
    v->data = malloc(v->item_size * v->capacity_items);
}

void vector_free(Vector *v) {
    free(v->data);
}

void *vector_push(Vector *v) {
    if (v->len >= v->capacity_items) {
        v->capacity_items <<= 1;
        v->data = realloc(v->data, v->item_size * v->capacity_items);
    }

    return (void *)((uint8_t *)v->data + v->item_size * v->len++);
}

void *vector_push_zeroed(Vector *v) {
    void *item = vector_push(v);
    memset(item, 0, v->item_size);
    return item;
}

void vector_pop(Vector *v) {
    v->len--;
}

void *vector_last(Vector *v) {
    return vector_get(v, v->len - 1);
}

void vector_clear(Vector *v) {
    v->len = 0;
}

void *vector_get(Vector *v, size_t index) {
    return (void *)((uint8_t *)v->data + index * v->item_size);
}
