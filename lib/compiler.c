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
    block_init((Block *)stack_push(&c->blocks));
    return c->blocks.size - 1;
}

static inline block_id begin_block(Compiler *c) {
    frame *f = (frame *)stack_push(&c->frames);
    f->block = push_block(c);
    return f->block;
}

/*
 * NOTE: Intentionally not returning the current frame's `block_id` because it
 *       may have changed since the last call to `begin_block`
 */
static inline void end_block(Compiler *c) {
    stack_pop(&c->frames);
}

static inline void get_current(Compiler *c, Block **b, frame **f) {
    frame *current_frame = (frame *)stack_top(&c->frames);
    if (f) *f = current_frame;
    if (b) *b = (Block *)stack_get(&c->blocks, current_frame->block);
}

static inline void put_jump(Compiler *c, opcode op, block_id block) {
    Block *b;
    frame *f;
    get_current(c, &b, &f);

    jump *j = (jump *)stack_push(&c->jumps);
    j->from_block = f->block;
    j->to_block = block;
    bytecode_put_jump(&b->bytecode, op, &j->addr_offset);
}

void compiler_init(Compiler *c) {
    buffer_init(&c->string_buffer);
    stack_init(&c->blocks, Block);
    stack_init(&c->frames, frame);
    stack_init(&c->jumps, jump);
    stack_init(&c->traces, trace);

    begin_block(c);
}

void compiler_free(Compiler *c) {
    buffer_free(&c->string_buffer);

    for (stack_iter i = stack_iter_begin(&c->blocks); stack_iter_next(&i);)
        block_free((Block *)i.item);

    stack_free(&c->blocks);
    stack_free(&c->frames);
    stack_free(&c->jumps);
    stack_free(&c->traces);
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

    size_t offset, len;
    compile_string(token_string(n->token), &c->string_buffer, &offset, &len);

    Block *b;
    get_current(c, &b, NULL);
    bytecode_put_pushstr(&b->bytecode, offset, len);
}

static void compiler_visit_ident(Compiler *c, node *n) {
    // TODO: Scope/name resolution

    size_t offset = c->string_buffer.len;
    buffer_puts(&c->string_buffer, token_string(n->token));
    uint16_t trace_id = c->traces.size;
    trace *t = (trace *)stack_push(&c->traces);
    t->string_offset = offset;
    t->string_len = n->token.len;

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

    bytecode_put_u16be(b, (uint16_t)c->traces.size);

    for (stack_iter i = stack_iter_begin(&c->traces); stack_iter_next(&i);) {
        trace *t = (trace *)i.item;
        bytecode_put_u32be(b, t->string_offset);
        bytecode_put_u32be(b, t->string_len);
    }

    bytecode_put_u32be(b, (uint32_t)c->string_buffer.len);
    buffer_puts(b, buffer_string(&c->string_buffer));

    size_t block_offset = 0;
    for (stack_iter i = stack_iter_begin(&c->blocks); stack_iter_next(&i);) {
        Block *block = (Block *)i.item;
        block->offset = block_offset;
        block_offset += block->bytecode.len;
    }

    for (stack_iter i = stack_iter_begin(&c->jumps); stack_iter_next(&i);) {
        jump *j = (jump *)i.item;
        Block *from_block = (Block *)stack_get(&c->blocks, j->from_block);
        Block *to_block = (Block *)stack_get(&c->blocks, j->to_block);
        bytecode_set_u32be(from_block->bytecode.data + j->addr_offset, to_block->offset);
    }

    for (stack_iter i = stack_iter_begin(&c->blocks); stack_iter_next(&i);) {
        Block *block = (Block *)i.item;
        buffer_puts(b, buffer_string(&block->bytecode));
    }
}

void compile_string(string str, Buffer *out, size_t *offset, size_t *len) {
    debug_assert(str.len >= 2);

    if (offset) *offset = out->len;

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

    if (len) *len = out->len - *offset;
}
