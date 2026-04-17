#ifndef STACK_H_
#define STACK_H_

#include <stddef.h>

#define stack_init(stack, item_type) \
    stack_init_(stack, sizeof(item_type))
#define stack_init_block_size(stack, item_type, block_size) \
    stack_init_block_size_(stack, sizeof(item_type), block_size)

typedef struct stack_block stack_block;

/**
 * A growable, homogenous LIFO container, pinned in memory
 */
typedef struct {
    size_t       item_size;
    size_t       items_per_block;
    size_t       size;
    stack_block *head;
} Stack;

void      stack_init_(Stack *, size_t item_size);
void      stack_init_block_size_(Stack *, size_t item_size, size_t items_per_block);
void      stack_free(Stack *);
void     *stack_push(Stack *);
void     *stack_push_zeroed(Stack *);
void      stack_pop(Stack *);
void     *stack_get(Stack *, size_t index);
void     *stack_top(Stack *);
void      stack_truncate(Stack *, size_t size);
void      stack_clear(Stack *);
size_t    stack_count_blocks(Stack *);

#endif // STACK_H_
