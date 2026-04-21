#include <string.h>

#include "bytecode.h"
#include "compiler.h"
#include "utils.h"

/*
 * A logical block of bytecode, either a branch or a function
 */
typedef struct {
    Buffer bytecode;
    size_t offset;
} Block;

typedef size_t block_id;

typedef struct {
    block_id block;
} frame;

/*
 * A reference to an instruction address stored in the bytecode that needs to
 * be resolved once all procs are populated and have known offsets
 */
typedef struct {
    block_id from_block;
    block_id to_block;
    /*
     * byte offset of instruction address relative to start of `from_pid` proc
     */
    size_t addr_offset;
} jump;

static void block_init(Block *b) {
    buffer_init(&b->bytecode);
    b->offset = 0; // offset is only calcuated in `compiler_write_program`
}

static void block_free(Block *b) {
    buffer_free(&b->bytecode);
}

static inline block_id push_block(Compiler *c) {
    block_init((Block *)vector_push(&c->blocks));
    return c->blocks.len - 1;
}

static inline block_id begin_block(Compiler *c) {
    frame *f = (frame *)vector_push(&c->frames);
    f->block = push_block(c);
    return f->block;
}

/*
 * NOTE: Intentionally not returning the current frame's `block_id` because it
 *       may have changed since the last call to `begin_block`
 */
static inline void end_block(Compiler *c) {
    vector_pop(&c->frames);
}

static inline void get_current(Compiler *c, Block **b, frame **f) {
    frame *current_frame = (frame *)vector_last(&c->frames);
    if (f) *f = current_frame;
    if (b) *b = (Block *)vector_get(&c->blocks, current_frame->block);
}

static inline void put_jump(Compiler *c, opcode op, block_id block) {
    Block *b;
    frame *f;
    get_current(c, &b, &f);

    jump *j = (jump *)vector_push(&c->jumps);
    j->from_block = f->block;
    j->to_block = block;
    bytecode_put_jump(&b->bytecode, op, &j->addr_offset);
}

void compiler_init(Compiler *c) {
    buffer_init(&c->string_buffer);
    string_pool_init(&c->strings);
    vector_init(&c->blocks, Block);
    vector_init(&c->frames, frame);
    vector_init(&c->jumps, jump);
    vector_init(&c->traces, trace);

    begin_block(c);
}

void compiler_free(Compiler *c) {
    buffer_free(&c->string_buffer);
    string_pool_free(&c->strings);

    for (EACH_IN_VECTOR(c->blocks, Block, block)) block_free(block);

    vector_free(&c->blocks);
    vector_free(&c->frames);
    vector_free(&c->jumps);
    vector_free(&c->traces);
}

static void compiler_visit(Compiler *c, node *n);

static void compiler_visit_int(Compiler *c, node *n) {
    uint64_t value = 0;
    for (size_t i = 0; i < n->token.len; i++)
        value = (value * 10) + (n->token.str[i] - '0');

    Block *b;
    get_current(c, &b, NULL);
    bytecode_put_pushint(&b->bytecode, value);
}

static void compiler_visit_string(Compiler *c, node *n) {
    // TODO: String interning

    compile_string(token_string(n->token), &c->string_buffer);
    symbol s = string_pool_intern(&c->strings, buffer_string(&c->string_buffer));
    string_pool_entry *e = (string_pool_entry *)vector_get(&c->strings.entries, s);
    buffer_clear(&c->string_buffer);

    Block *b;
    get_current(c, &b, NULL);
    bytecode_put_pushstr(&b->bytecode, e->offset, e->len);
}

static void compiler_visit_ident(Compiler *c, node *n) {
    // TODO: Scope/name resolution

    symbol s = string_pool_intern(&c->strings, token_string(n->token));
    string_pool_entry *e = (string_pool_entry *)vector_get(&c->strings.entries, s);

    uint16_t trace_id = c->traces.len;
    trace *t = (trace *)vector_push(&c->traces);
    t->string_offset = e->offset;
    t->string_len = e->len;

    Block *b;
    get_current(c, &b, NULL);
    bytecode_put_trace(&b->bytecode, trace_id);
}

static void compiler_visit_binary(Compiler *c, node *n) {
    for (node *child = n->first_child; child; child = child->next_sibling)
        compiler_visit(c, child);

    Block *b;
    get_current(c, &b, NULL);

    if (string_eq(token_string(n->token), STRING("+")))
        buffer_putc(&b->bytecode, OP_ADD);

    // TODO: Other built-ins and user functions
}

static void compiler_visit_if(Compiler *c, node *n) {
    node *cond = n->first_child;
    node *then = cond->next_sibling;
    node *elze = then->next_sibling;

    frame *f;
    get_current(c, NULL, &f);

    // prepare a continuation block so that the `elze` block falls through to it
    block_id cont_block = push_block(c);

    block_id then_block = begin_block(c);
    compiler_visit(c, then);
    put_jump(c, OP_JUMP, cont_block);
    end_block(c);

    compiler_visit(c, cond);
    put_jump(c, OP_JUMPIF, then_block);
    compiler_visit(c, elze);

    f->block = cont_block; // resume in continuation block
}

static void compiler_visit_doblk(Compiler *c, node *n) {
    // TODO: Handle statements
    node *child = n->first_child;
    while (child->next_sibling) child = child->next_sibling;
    compiler_visit(c, child);
}

static void compiler_visit(Compiler *c, node *n) {
    switch (n->type) {
    case N_INT: compiler_visit_int(c, n); break;
    case N_STRING: compiler_visit_string(c, n); break;
    case N_IDENT: compiler_visit_ident(c, n); break;
    case N_BINARY: compiler_visit_binary(c, n); break;
    case N_IF: compiler_visit_if(c, n); break;
    case N_DOBLK: compiler_visit_doblk(c, n); break;
    default:
        panic("unsupported node: %s", node_type_name(n->type));
    }
}

void compiler_visit_program(Compiler *c, node *n) {
    compiler_visit(c, n);

    Block *b;
    get_current(c, &b, NULL);
    buffer_putc(&b->bytecode, (char)OP_HALT);
}

void compiler_write_program(Compiler *c, Buffer *b) {
    buffer_puts(b, STRING(MAGIC_HAMMER));
    bytecode_put_u16be(b, BYTECODE_VERSION);

    bytecode_put_u16be(b, (uint16_t)c->traces.len);

    for (EACH_IN_VECTOR(c->traces, trace, t)) {
        bytecode_put_u32be(b, t->string_offset);
        bytecode_put_u32be(b, t->string_len);
    }

    bytecode_put_u32be(b, (uint32_t)c->strings.buffer.len);
    buffer_puts(b, buffer_string(&c->strings.buffer));

    size_t block_offset = 0;
    for (EACH_IN_VECTOR(c->blocks, Block, block)) {
        block->offset = block_offset;
        block_offset += block->bytecode.len;
    }

    for (EACH_IN_VECTOR(c->jumps, jump, j)) {
        Block *from_block = (Block *)vector_get(&c->blocks, j->from_block);
        Block *to_block = (Block *)vector_get(&c->blocks, j->to_block);
        bytecode_set_u32be(from_block->bytecode.data + j->addr_offset, to_block->offset);
    }

    for (EACH_IN_VECTOR(c->blocks, Block, block))
        buffer_puts(b, buffer_string(&block->bytecode));
}

void compile_string(string str, Buffer *out) {
    debug_assert(str.len >= 2);

    bool escape = false;
    for (size_t i = 1; i < str.len - 1; i++) { // ignore quotes
        if (str.data[i] == '\\' && !escape) {
            escape = true;
            continue;
        }

        if (escape) {
            switch (str.data[i]) {
            case 'n': buffer_putc(out, '\n'); break;
            case 'r': buffer_putc(out, '\r'); break;
            // TODO: Add other escapes
            default:
                buffer_putc(out, str.data[i]);
            }

            escape = false;
        } else {
            buffer_putc(out, str.data[i]);
        }
    }
}
