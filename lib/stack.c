#include "stack.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define STACK_DEFAULT_BLOCK_SIZE 0x400

struct stack_block {
    stack_block *next;
    char         data[];
};

static stack_block *stack_block_new(size_t size) {
    debug_assert(size > sizeof(stack_block));

    stack_block *block = malloc(size);
    block->next = NULL;
    return block;
}

static inline size_t stack_block_capacity(Stack *s) {
    return s->block_size - sizeof(stack_block);
}

void stack_init(Stack *s) {
    stack_init_block_size(s, STACK_DEFAULT_BLOCK_SIZE);
}

void stack_init_block_size(Stack *s, size_t block_size) {
    s->block_size = block_size;
    s->cursor = 0;
    s->head = stack_block_new(s->block_size);
}

void stack_free(Stack *s) {
    stack_block *block = s->head;

    while (block) {
        stack_block *next = block->next;
        free(block);
        block = next;
    }
}

void *stack_push_(Stack *s, size_t size) {
    size_t capacity = stack_block_capacity(s);
    debug_assert(size <= capacity);

    size_t       offset = s->cursor;
    stack_block *block = s->head;

    while (offset >= capacity) {
        block = block->next;
        offset -= capacity;
    }

    if (size > capacity - offset) {
        if (!block->next) block->next = stack_block_new(s->block_size);
        block = block->next;

        s->cursor += capacity - offset;
        offset = 0;
    }

    void *ptr = block->data + offset;
    s->cursor += size;
    return ptr;
}

void *stack_push_zeroed_(Stack *s, size_t size) {
    void *ptr = stack_push_(s, size);
    memset(ptr, 0, size);
    return ptr;
}

stack_ptr stack_top(Stack *s) {
    return (stack_ptr)s->cursor;
}

void stack_rewind(Stack *s, stack_ptr cursor) {
    debug_assert((size_t)cursor <= s->cursor);

    s->cursor = (size_t)cursor;
}

void stack_clear(Stack *s) {
    s->cursor = 0;
}
