#include <string.h>

#include "bytecode.h"
#include "compiler.h"
#include "utils.h"

void compiler_init(Compiler *c) {
    buffer_init(&c->string_buffer);
    buffer_init(&c->bytecode);
}

void compiler_free(Compiler *c) {
    buffer_free(&c->string_buffer);
    buffer_free(&c->bytecode);
}

static void compiler_visit_string(Compiler *c, node *n) {
    // TODO: Unescape string
    // TODO: String interning
    size_t len = n->token.len;
    char *data = buffer_alloc(&c->string_buffer, len);
    memcpy(data, n->token.str, len);
    size_t offset = (size_t)(data - c->string_buffer.data);

    buffer_write_u8be(&c->bytecode, OP_PUSHSTR);
    buffer_write_u32be(&c->bytecode, (uint32_t)offset);
    buffer_write_u32be(&c->bytecode, (uint32_t)len);
}

static void compiler_visit_binary(Compiler *c, node *n) {
    for (node *child = n->first_child; child; child = child->next_sibling)
        compiler_visit(c, child);

    if (string_eq(token_string(n->token), STRING("+")))
        buffer_write_u8be(&c->bytecode, OP_ADD);

    // TODO: Other built-ins and user functions
}

static void compiler_visit_doblk(Compiler *c, node *n) {
    // TODO: Handle statements
    node *child = n->first_child;
    while (child->next_sibling) child = child->next_sibling;
    compiler_visit(c, child);
}

void compiler_visit(Compiler *c, node *n) {
    switch (n->type) {
    case N_STRING: compiler_visit_string(c, n); break;
    case N_BINARY: compiler_visit_binary(c, n); break;
    case N_DOBLK: compiler_visit_doblk(c, n); break;
    default:
        panic("unsupported node: %s", node_type_name(n->type));
    }
}

void compiler_write_program(Compiler *c, Buffer *b) {
    buffer_puts(b, MAGIC_HAMMER, sizeof(MAGIC_HAMMER) - 1);
    buffer_write_u16be(b, BYTECODE_VERSION);
    buffer_write_u16be(b, 0); // trace table length
    buffer_write_u32be(b, (uint32_t)c->string_buffer.len);
    buffer_puts(b, c->string_buffer.data, c->string_buffer.len);
    buffer_puts(b, c->bytecode.data, c->bytecode.len);
}
