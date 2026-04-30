#include "../lib/vector.h"
#include "lib/test.h"

TEST(vector) {
    Vector v;
    vector_init_capacity(&v, uint32_t, 2);

    *(uint32_t *)vector_push(&v) = 1;
    ASSERT_INT_EQ(v.len, 1);
    ASSERT_INT_EQ(v.capacity_items, 2);

    *(uint32_t *)vector_push(&v) = 2;
    ASSERT_INT_EQ(v.len, 2);
    ASSERT_INT_EQ(v.capacity_items, 2);

    *(uint32_t *)vector_push(&v) = 3;
    ASSERT_INT_EQ(v.len, 3);
    ASSERT_INT_EQ(v.capacity_items, 4);

    uint32_t sum = 0;
    for (size_t i = 0; i < v.len; i++) sum += *(uint32_t *)vector_get(&v, i);
    ASSERT_INT_EQ(sum, 6);

    sum = 0;
    for (EACH_IN_VECTOR(v, uint32_t, i)) sum += *i;
    ASSERT_INT_EQ(sum, 6);

    uint32_t popped;
    ASSERT(vector_pop(&v, &popped));
    ASSERT_INT_EQ(popped, 3);
    ASSERT(vector_pop(&v, &popped));
    ASSERT_INT_EQ(popped, 2);
    ASSERT(vector_pop(&v, &popped));
    ASSERT_INT_EQ(popped, 1);
    ASSERT(!vector_pop(&v, &popped));

    vector_free(&v);
    return TEST_OK;
}
