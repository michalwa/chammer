#include "lib/test.h"
#include "../lib/vector.h"

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
    for (size_t i = 0; i < v.len; i++) {
        sum += *(uint32_t *)vector_get(&v, i);
    }
    ASSERT_INT_EQ(sum, 6);

    vector_free(&v);
    return TEST_OK;
}
