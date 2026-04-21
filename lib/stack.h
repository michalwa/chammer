#ifndef STACK_H_
#define STACK_H_

#include <stdbool.h>
#include <stddef.h>

#define stack_init(stack, item_type) \
    stack_init_(stack, sizeof(item_type))
#define stack_init_block_size(stack, item_type, block_size) \
    stack_init_block_size_(stack, sizeof(item_type), block_size)

/**
 * For use in for-loops
 */
#define STACK_ITER(stack, var, type) \
    type *var = stack_iter_begin(&(stack)); \
    var; \
    var = stack_iter_next(&(stack), )

typedef struct stack_block stack_block;

/**
 * Like a Vector, but pinned in memory so as not to invalidate pointers.
 * Less efficient than a Vector if pinning is not needed.
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
