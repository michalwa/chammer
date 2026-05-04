#include "../lib/arena.h"
#include "lib/test.h"

TEST(arena) {
    Arena arena;
    arena_init_block_size(&arena, uint32_t, 2);

    *(uint32_t *)arena_push(&arena) = 42;
    ASSERT_INT_EQ(arena.len, 1);

    *(uint32_t *)arena_push(&arena) = 43;
    ASSERT_INT_EQ(arena.len, 2);
    ASSERT_INT_EQ(arena_count_blocks(&arena), 1);
    ASSERT_INT_EQ(*(uint32_t *)arena_head(&arena), 42);

    *(uint32_t *)arena_push(&arena) = 44;
    ASSERT_INT_EQ(arena.len, 3);
    ASSERT_INT_EQ(arena_count_blocks(&arena), 2);

    uint32_t sum = 0;
    for (arena_iter i = arena_iter_begin(&arena); arena_iter_next(&i);) sum += *(uint32_t *)i.item;
    ASSERT_INT_EQ(sum, 42 + 43 + 44);

    arena_truncate(&arena, 0);
    ASSERT_INT_EQ(arena.len, 0);

    *(uint32_t *)arena_push(&arena) = 45;
    ASSERT_INT_EQ(arena.len, 1);
    ASSERT_INT_EQ(*(uint32_t *)arena_head(&arena), 45);

    arena_free(&arena);

    return TEST_OK;
}
