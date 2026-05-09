#include <stdlib.h>

#include "../lib/utils.h"
#include "../lib/value.h"
#include "lib/test.h"

typedef struct {
    int64_t value;
} HNativeTestDummy;

static int test_dummy_free_count;
static int test_dummy_clone_count;

static void reset_counts(void) {
    test_dummy_free_count = 0;
    test_dummy_clone_count = 0;
}

static void test_dummy_free(void *data) {
    test_dummy_free_count++;
    free(data);
}

static void *test_dummy_clone(const void *data) {
    test_dummy_clone_count++;
    return memcpy(malloc(sizeof(HNativeTestDummy)), data, sizeof(HNativeTestDummy));
}

static HValue test_dummy_call(const void *data, const HValue *args, Machine *ctx) {
    (void)ctx;
    assert(args[0].type == V_INT);
    int64_t result = ((HNativeTestDummy *)data)->value + args[0].v_int;
    return hvalue_make_int(result);
}

static hnative_meta HNATIVE_TEST_DUMMY_META = {
    .name = "test_dummy",
    .argc = 1,
    .free = test_dummy_free,
    .clone = test_dummy_clone,
    .call = test_dummy_call,
};

static HValue hnative_make_test_dummy(int64_t value) {
    HNativeTestDummy *data = malloc(sizeof(*data));
    data->value = value;
    return hvalue_make_native(&HNATIVE_TEST_DUMMY_META, data);
}

TEST(hvalue_ownership) {
    HValue a, b;

    reset_counts();

    a = hnative_make_test_dummy(1);
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

    a = hnative_make_test_dummy(1);
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

    a = hnative_make_test_dummy(1);
    a = hvalue_uniq(a);
    ASSERT_INT_EQ(test_dummy_clone_count, 0);

    b = hvalue_ref(&a);
    c = hvalue_uniq(a);
    ASSERT_INT_EQ(test_dummy_clone_count, 1);

    hvalue_drop(b);
    hvalue_drop(c);

    return TEST_OK;
}

TEST(hvalue_string) {
    HValue hv = hvalue_make_string(STRING("Hello, world!"));
    ASSERT_ENUM_EQ(hv.type, V_STRING, hvalue_type_name);
    ASSERT_STRING_EQ(hvalue_string_get(&hv), STRING("Hello, world!"));

    HValue hv_same = hvalue_make_string(STRING("Hello, world!"));
    HValue hv_diff = hvalue_make_string(STRING("Hello"));

    ASSERT(hvalue_eq(&hv, &hv));
    ASSERT(hvalue_eq(&hv, &hv_same));
    ASSERT(!hvalue_eq(&hv, &hv_diff));

    hvalue_drop(hv);
    hvalue_drop(hv_same);
    hvalue_drop(hv_diff);

    return TEST_OK;
}

TEST(hvalue_substr) {
    HValue hv_string = hvalue_make_string(STRING("Hello, world!"));
    HValue hv_hello = hvalue_make_substr(hvalue_ref(&hv_string), 0, 5);
    HValue hv_world = hvalue_make_substr(hvalue_ref(&hv_string), 7, 5);

    ASSERT_ENUM_EQ(hv_hello.type, V_SUBSTR, hvalue_type_name);

    ASSERT_STRING_EQ(hvalue_string_get(&hv_hello), STRING("Hello"));
    ASSERT_STRING_EQ(hvalue_string_get(&hv_world), STRING("world"));

    HValue hv_subsub = hvalue_make_substr(hvalue_ref(&hv_hello), 1, 100);
    ASSERT_STRING_EQ(hvalue_string_get(&hv_subsub), STRING("ello"));

    HValue hv_subsubsub = hvalue_make_substr(hvalue_ref(&hv_subsub), 1, 100);
    ASSERT_STRING_EQ(hvalue_string_get(&hv_subsubsub), STRING("llo"));

    hvalue_drop(hv_subsubsub);
    hvalue_drop(hv_subsub);

    hv_subsub = hvalue_make_substr(hvalue_ref(&hv_hello), 3, 100);
    ASSERT_STRING_EQ(hvalue_string_get(&hv_subsub), STRING("lo"));
    hvalue_drop(hv_subsub);

    HValue hv_full = hvalue_make_substr(hvalue_ref(&hv_string), 0, 100);

    ASSERT(hvalue_eq(&hv_hello, &hv_hello));
    ASSERT(hvalue_eq(&hv_string, &hv_full));
    ASSERT(!hvalue_eq(&hv_hello, &hv_world));

    hvalue_drop(hv_full);

    HValue hv_out_of_bounds = hvalue_make_substr(hvalue_ref(&hv_string), 100, 5);
    ASSERT_STRING_EQ(hvalue_string_get(&hv_out_of_bounds), STRING(""));
    hvalue_drop(hv_subsub);

    hv_out_of_bounds = hvalue_make_substr(hvalue_ref(&hv_hello), 100, 5);
    ASSERT_STRING_EQ(hvalue_string_get(&hv_out_of_bounds), STRING(""));
    hvalue_drop(hv_subsub);

    ASSERT(!hvalue_is_uniq(&hv_string));

    hvalue_drop(hv_hello);
    hvalue_drop(hv_world);

    ASSERT(hvalue_is_uniq(&hv_string));

    hvalue_drop(hv_string);

    return TEST_OK;
}

TEST(hvalue_closure) {
    HValue hv = hvalue_make_closure(1, 2);
    ASSERT_ENUM_EQ(hv.type, V_CLOSURE, hvalue_type_name);
    ASSERT_INT_EQ(hvalue_closure_fnindex(&hv), 1);
    ASSERT_INT_EQ(hvalue_closure_args_left(&hv), 2);

    hvalue_closure_put_arg_mut(&hv, hvalue_make_int(42));
    ASSERT_INT_EQ(hvalue_closure_args_left(&hv), 1);

    HValue arg;
    ASSERT(hvalue_closure_take_arg_mut(&hv, &arg));
    ASSERT_ENUM_EQ(arg.type, V_INT, hvalue_type_name);
    ASSERT_INT_EQ(arg.v_int, 42);
    hvalue_drop(arg);
    ASSERT(!hvalue_closure_take_arg_mut(&hv, &arg));

    reset_counts();

    hvalue_closure_put_arg_mut(&hv, hnative_make_test_dummy(1));
    ASSERT_INT_EQ(hvalue_closure_args_left(&hv), 1);
    hvalue_closure_put_arg_mut(&hv, hnative_make_test_dummy(2));
    ASSERT_INT_EQ(hvalue_closure_args_left(&hv), 0);
    hvalue_drop(hv);

    ASSERT_INT_EQ(test_dummy_free_count, 2);

    return TEST_OK;
}

TEST(hvalue_cons) {
    HValue a, b;
    a = hvalue_make_cons(
        hvalue_make_int(1), hvalue_make_cons(hvalue_make_int(2), hvalue_make(V_NIL))
    );

    hvalue_uncons(a, &a, &b);
    ASSERT_ENUM_EQ(a.type, V_INT, hvalue_type_name);
    ASSERT_INT_EQ(a.v_int, 1);
    ASSERT_ENUM_EQ(b.type, V_CONS, hvalue_type_name);

    hvalue_uncons(b, &a, &b);
    ASSERT_ENUM_EQ(a.type, V_INT, hvalue_type_name);
    ASSERT_INT_EQ(a.v_int, 2);
    ASSERT_ENUM_EQ(b.type, V_NIL, hvalue_type_name);

    hvalue_drop(a);
    hvalue_drop(b);

    a = hvalue_make_cons(hvalue_make_int(1), hvalue_make(V_NIL));
    b = hvalue_make_cons(hvalue_make_int(2), hvalue_make(V_NIL));
    a = hvalue_list_concat(a, b);
    ASSERT_ENUM_EQ(a.type, V_CONS, hvalue_type_name);
    hvalue_uncons(a, &a, &b);
    ASSERT_ENUM_EQ(a.type, V_INT, hvalue_type_name);
    ASSERT_INT_EQ(a.v_int, 1);
    ASSERT_ENUM_EQ(b.type, V_CONS, hvalue_type_name);
    hvalue_uncons(b, &a, &b);
    ASSERT_ENUM_EQ(a.type, V_INT, hvalue_type_name);
    ASSERT_INT_EQ(a.v_int, 2);
    ASSERT_ENUM_EQ(b.type, V_NIL, hvalue_type_name);

    hvalue_drop(a);
    hvalue_drop(b);

    reset_counts();

    a = hvalue_make_cons(
        hnative_make_test_dummy(1), hvalue_make_cons(hnative_make_test_dummy(2), hvalue_make(V_NIL))
    );
    hvalue_drop(a);

    ASSERT_INT_EQ(test_dummy_free_count, 2);

    return TEST_OK;
}

TEST(hvalue_tuple) {
    HTupleBuilder tb = htuple_begin(3);
    htuple_put(&tb, hvalue_make_int(1));
    htuple_put(&tb, hvalue_make_int(2));
    htuple_put(&tb, hvalue_make_int(3));
    HValue hv = htuple_end(tb);

    ASSERT_ENUM_EQ(hv.type, V_TUPLE, hvalue_type_name);
    ASSERT_INT_EQ(hvalue_tuple_len(&hv), 3);

    for (int i = 0; i < 3; i++) {
        const HValue item = hvalue_tuple_get(&hv, i);
        ASSERT_ENUM_EQ(item.type, V_INT, hvalue_type_name);
        ASSERT_INT_EQ(item.v_int, i + 1);
        hvalue_drop(item);
    }

    hvalue_drop(hv);

    reset_counts();

    tb = htuple_begin(2);
    htuple_put(&tb, hnative_make_test_dummy(1));
    htuple_put(&tb, hnative_make_test_dummy(2));
    hv = htuple_end(tb);
    hvalue_drop(hv);

    ASSERT_INT_EQ(test_dummy_free_count, 2);

    tb = htuple_begin(0);
    hv = htuple_end(tb);
    ASSERT_ENUM_EQ(hv.type, V_TUPLE, hvalue_type_name);
    ASSERT_INT_EQ(hvalue_tuple_len(&hv), 0);

    HValue ref = hvalue_ref(&hv);
    ASSERT(hvalue_is_uniq(&hv));
    hvalue_drop(ref);

    hvalue_drop(hv);

    return TEST_OK;
}

TEST(hvalue_native) {
    HValue hv = hnative_make_test_dummy(40);
    ASSERT_ENUM_EQ(hv.type, V_NATIVE, hvalue_type_name);
    ASSERT_INT_EQ(hvalue_native_args_left(&hv), 1);

    hvalue_native_put_arg_mut(&hv, hvalue_make_int(2));
    ASSERT_INT_EQ(hvalue_native_args_left(&hv), 0);

    HValue result = hvalue_native_call(&hv, NULL);
    ASSERT_ENUM_EQ(result.type, V_INT, hvalue_type_name);
    ASSERT_INT_EQ(result.v_int, 42);

    reset_counts();

    hvalue_drop(hv);
    hvalue_drop(result);

    ASSERT_INT_EQ(test_dummy_free_count, 1);

    return TEST_OK;
}
