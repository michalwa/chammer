#include "../lib/value.h"
#include "lib/test.h"

static int test_dummy_free_count;
static int test_dummy_clone_count;

static void reset_counts(void) {
    test_dummy_free_count = 0;
    test_dummy_clone_count = 0;
}

static void test_dummy_free(void *data) {
    (void)data;
    test_dummy_free_count++;
}

static void *test_dummy_clone(const void *data) {
    (void)data;
    test_dummy_clone_count++;
    return NULL;
}

static hnative_meta HNATIVE_TEST_DUMMY_META = {
    .name = "test_dummy",
    .free = test_dummy_free,
    .clone = test_dummy_clone,
};

static HValue hnative_make_test_dummy(void) {
    return hvalue_make_native(&HNATIVE_TEST_DUMMY_META, NULL);
}

TEST(hvalue_ownership) {
    HValue a, b;

    reset_counts();

    a = hnative_make_test_dummy();
    ASSERT(hvalue_is_uniq(&a));

    b = hvalue_ref(&a);
    ASSERT(!hvalue_is_uniq(&a));
    ASSERT(!hvalue_is_uniq(&b));

    hvalue_drop(a);
    ASSERT_INT_EQ(test_dummy_free_count, 0);
    ASSERT(hvalue_is_uniq(&b));

    hvalue_drop(b);
    ASSERT_INT_EQ(test_dummy_free_count, 1);

    return TEST_OK;
}

TEST(hvalue_clone) {
    HValue a, b;

    reset_counts();

    a = hnative_make_test_dummy();
    b = hvalue_clone(&a);
    ASSERT_INT_EQ(test_dummy_clone_count, 1);
    ASSERT(hvalue_is_uniq(&a));
    ASSERT(hvalue_is_uniq(&b));

    hvalue_drop(a);
    ASSERT_INT_EQ(test_dummy_free_count, 1);

    hvalue_drop(b);
    ASSERT_INT_EQ(test_dummy_free_count, 2);

    return TEST_OK;
}

TEST(hvalue_uniq) {
    HValue a, b, c;

    reset_counts();

    a = hnative_make_test_dummy();
    a = hvalue_uniq(a);
    ASSERT_INT_EQ(test_dummy_clone_count, 0);

    b = hvalue_ref(&a);
    c = hvalue_uniq(a);
    ASSERT_INT_EQ(test_dummy_clone_count, 1);

    hvalue_drop(b);
    hvalue_drop(c);

    return TEST_OK;
}
