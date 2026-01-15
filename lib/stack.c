#include "stack.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define STACK_DEFAULT_BLOCK_SIZE 0x400

struct stack_block {
    char        *data;
    stack_block *next;
};

static stack_block *stack_block_new(size_t size) {
    stack_block *block = malloc(sizeof(stack_block));
    block->data = malloc(size);
    block->next = NULL;
    return block;
}

static void stack_block_free(stack_block *block) {
    free(block->data);
    free(block);
}

void stack_init(Stack *s) {
    stack_init_block_size(s, STACK_DEFAULT_BLOCK_SIZE);
}

void stack_init_block_size(Stack *s, size_t block_size) {
    s->block_size = block_size;
    s->cursor = 0;
    s->head = stack_block_new(s->block_size);
}

void *stack_push_(Stack *s, size_t size) {
    if (size > s->block_size) {
        fprintf(stderr, "WARN: `stack_push` called with `size > block_size`\n");
        return NULL;
    }

    size_t       offset = s->cursor;
    stack_block *block = s->head;

    while (offset >= s->block_size) {
        block = block->next;
        offset -= s->block_size;
    }

    if (size > s->block_size - offset) {
        block->next = stack_block_new(s->block_size);
        block = block->next;

        s->cursor += s->block_size - offset;
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
    if ((size_t)cursor > s->cursor) {
        fprintf(stderr, "WARN: `stack_rewind` called with `stack_ptr > top`\n");
        return;
    }

    s->cursor = (size_t)cursor;
}

void stack_free(Stack *s) {
    stack_block *block = s->head;

    while (block) {
        stack_block *next = block->next;
        stack_block_free(block);
        block = next;
    }
}
