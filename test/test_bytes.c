#include "../lib/bytes.h"
#include "../lib/string.h"
#include "lib/test.h"

TEST(read_be_bytes) {
    const uint8_t *b;

    b = (uint8_t[3]){ 0x12, 0x34, 0xFF };
    ASSERT_INT_EQ(0x1234, read_u16be(&b));
    ASSERT_INT_EQ(*b, 0xFF);

    b = (uint8_t[5]){ 0x12, 0x34, 0x56, 0x78, 0xFF };
    ASSERT_INT_EQ(0x12345678, read_u32be(&b));
    ASSERT_INT_EQ(*b, 0xFF);

    b = (uint8_t[9]){ 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0xFF };
    ASSERT_INT_EQ(0x123456789ABCDEF0, read_u64be(&b));
    ASSERT_INT_EQ(*b, 0xFF);

    b = (uint8_t[3]){ 0x12, 0x34, 0xFF };
    ASSERT_INT_EQ((int16_t)0x1234, read_i16be(&b));
    ASSERT_INT_EQ(*b, 0xFF);

    b = (uint8_t[5]){ 0x12, 0x34, 0x56, 0x78, 0xFF };
    ASSERT_INT_EQ((int32_t)0x12345678, read_i32be(&b));
    ASSERT_INT_EQ(*b, 0xFF);

    b = (uint8_t[9]){ 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0xFF };
    ASSERT_INT_EQ((int64_t)0x123456789ABCDEF0, read_i64be(&b));
    ASSERT_INT_EQ(*b, 0xFF);

    return TEST_OK;
}

TEST(write_be_bytes) {
    uint8_t  buffer[9] = { 0 };
    uint8_t *b;

    b = memset(buffer, 0xFF, sizeof(buffer));
    write_u16be(&b, 0x1234);
    ASSERT(memcmp(buffer, (uint8_t[]){ 0x12, 0x34 }, 2) == 0);
    ASSERT_INT_EQ(*b, 0xFF);

    b = memset(buffer, 0xFF, sizeof(buffer));
    write_u32be(&b, 0x12345678);
    ASSERT(memcmp(buffer, (uint8_t[]){ 0x12, 0x34, 0x56, 0x78 }, 4) == 0);
    ASSERT_INT_EQ(*b, 0xFF);

    b = memset(buffer, 0xFF, sizeof(buffer));
    write_u64be(&b, 0x123456789ABCDEF0);
    ASSERT(memcmp(buffer, (uint8_t[]){ 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 }, 8) == 0);
    ASSERT_INT_EQ(*b, 0xFF);

    b = memset(buffer, 0xFF, sizeof(buffer));
    write_i16be(&b, (int16_t)0x1234);
    ASSERT(memcmp(buffer, (uint8_t[]){ 0x12, 0x34 }, 2) == 0);
    ASSERT_INT_EQ(*b, 0xFF);

    b = memset(buffer, 0xFF, sizeof(buffer));
    write_i32be(&b, (int32_t)0x12345678);
    ASSERT(memcmp(buffer, (uint8_t[]){ 0x12, 0x34, 0x56, 0x78 }, 4) == 0);
    ASSERT_INT_EQ(*b, 0xFF);

    b = memset(buffer, 0xFF, sizeof(buffer));
    write_i64be(&b, (int64_t)0x123456789ABCDEF0);
    ASSERT(memcmp(buffer, (uint8_t[]){ 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 }, 8) == 0);
    ASSERT_INT_EQ(*b, 0xFF);

    return TEST_OK;
}

TEST(buffer_put_be_bytes) {
    Buffer b;
    buffer_init_capacity(&b, 9);

    buffer_put_u16be(&b, 0x1234);
    ASSERT(memcmp(b.data, (uint8_t[]){ 0x12, 0x34 }, 2) == 0);
    ASSERT_INT_EQ(b.len, 2);

    buffer_clear(&b);
    buffer_put_u32be(&b, 0x12345678);
    ASSERT(memcmp(b.data, (uint8_t[]){ 0x12, 0x34, 0x56, 0x78 }, 4) == 0);
    ASSERT_INT_EQ(b.len, 4);

    buffer_clear(&b);
    buffer_put_u64be(&b, 0x123456789ABCDEF0);
    ASSERT(memcmp(b.data, (uint8_t[]){ 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 }, 8) == 0);
    ASSERT_INT_EQ(b.len, 8);

    buffer_clear(&b);
    buffer_put_i16be(&b, (int16_t)0x1234);
    ASSERT(memcmp(b.data, (uint8_t[]){ 0x12, 0x34 }, 2) == 0);
    ASSERT_INT_EQ(b.len, 2);

    buffer_clear(&b);
    buffer_put_i32be(&b, (int32_t)0x12345678);
    ASSERT(memcmp(b.data, (uint8_t[]){ 0x12, 0x34, 0x56, 0x78 }, 4) == 0);
    ASSERT_INT_EQ(b.len, 4);

    buffer_clear(&b);
    buffer_put_i64be(&b, (int64_t)0x123456789ABCDEF0);
    ASSERT(memcmp(b.data, (uint8_t[]){ 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 }, 8) == 0);
    ASSERT_INT_EQ(b.len, 8);

    buffer_free(&b);

    return TEST_OK;
}
