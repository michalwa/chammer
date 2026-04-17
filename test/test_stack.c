#include "../lib/stack.h"
#include "lib/test.h"

TEST(stack) {
    Stack stack;
    stack_init_block_size(&stack, uint32_t, 2);

    *(uint32_t *)stack_push(&stack) = 42;
    ASSERT_INT_EQ(stack.size, 1);
    ASSERT_INT_EQ(*(uint32_t *)stack_top(&stack), 42);

    *(uint32_t *)stack_push(&stack) = 43;
    ASSERT_INT_EQ(stack.size, 2);
    ASSERT_INT_EQ(stack_count_blocks(&stack), 1);
    ASSERT_INT_EQ(*(uint32_t *)stack_top(&stack), 43);
    ASSERT_INT_EQ(*(uint32_t *)stack_get(&stack, 0), 42);

    *(uint32_t *)stack_push(&stack) = 44;
    ASSERT_INT_EQ(stack.size, 3);
    ASSERT_INT_EQ(stack_count_blocks(&stack), 2);
    ASSERT_INT_EQ(*(uint32_t *)stack_top(&stack), 44);

    stack_truncate(&stack, 0);
    ASSERT_INT_EQ(stack.size, 0);

    *(uint32_t *)stack_push(&stack) = 45;
    ASSERT_INT_EQ(stack.size, 1);
    ASSERT_INT_EQ(*(uint32_t *)stack_top(&stack), 45);
    ASSERT_INT_EQ(*(uint32_t *)stack_get(&stack, 0), 45);

    stack_free(&stack);

    return TEST_OK;
}
