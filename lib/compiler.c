#include "compiler.h"

#include <string.h>

#include "bytecode.h"
#include "utils.h"

/*
 * A logical block of bytecode, either a branch or a function
 */
typedef struct {
    Buffer bytecode;
    size_t offset;
} Block;

typedef size_t block_id;

typedef struct Frame Frame;
struct Frame {
    Frame   *prev;
    block_id block;
    Vector   locals;
};

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
    size_t   addr_offset;
} jump;

static inline void block_init(Block *b) {
    buffer_init(&b->bytecode);
    b->offset = 0; // offset is only calcuated in `compiler_write_program`
}

static inline void block_free(Block *b) {
    buffer_free(&b->bytecode);
}

static inline void frame_init(Frame *f, Frame *prev, block_id block) {
    f->prev = prev;
    f->block = block;
    vector_init(&f->locals, symbol);
}

static inline void frame_free(Frame *f) {
    vector_free(&f->locals);
}

void compiler_init(Compiler *c) {
    buffer_init(&c->string_buffer);
    string_pool_init(&c->strings);
    string_pool_init(&c->idents);
    vector_init(&c->blocks, Block);
    vector_init(&c->jumps, jump);
    vector_init(&c->traces, trace);
}

void compiler_free(Compiler *c) {
    for (EACH_IN_VECTOR(c->blocks, Block, block)) block_free(block);

    buffer_free(&c->string_buffer);
    string_pool_free(&c->strings);
    string_pool_free(&c->idents);
    vector_free(&c->blocks);
    vector_free(&c->jumps);
    vector_free(&c->traces);
}

static inline block_id push_block(Compiler *c) {
    block_init((Block *)vector_push(&c->blocks));
    return c->blocks.len - 1;
}

static inline Block *get_block(Compiler *c, Frame *f) {
    return (Block *)vector_get(&c->blocks, f->block);
}

static void put_jump(Compiler *c, Frame *f, opcode op, block_id block) {
    jump *j = (jump *)vector_push(&c->jumps);
    j->from_block = f->block;
    j->to_block = block;

    Block *b = get_block(c, f);
    bytecode_put_jump(&b->bytecode, op, &j->addr_offset);
}

/*
 * Resolves the name to a local ID without searching previous frames
 */
static bool get_strict_local(Frame *f, symbol name, uint8_t *id) {
    for (uint8_t i = 0; i < (uint8_t)f->locals.len; i++) {
        symbol *local = (symbol *)vector_get(&f->locals, i);
        if (name == *local) {
            *id = i;
            return true;
        }
    }

    return false;
}

static uint8_t get_or_insert_local(Frame *f, symbol name) {
    uint8_t id;
    if (get_strict_local(f, name, &id)) return id;

    *(symbol *)vector_push(&f->locals) = name;
    return (uint8_t)(f->locals.len - 1);
}

static void visit(Compiler *c, Frame *f, node *n);
static void visit_pattern(Compiler *c, Frame *f, node *lhs, node *rhs, node *cont);

static void visit_int(Compiler *c, Frame *f, node *n) {
    uint64_t value = 0;
    for (size_t i = 0; i < n->token.len; i++) value = (value * 10) + (n->token.str[i] - '0');

    Block *b = get_block(c, f);
    bytecode_put_pushint(&b->bytecode, value);
}

static void visit_string(Compiler *c, Frame *f, node *n) {
    compile_string(token_string(n->token), &c->string_buffer);
    symbol             s = string_pool_intern(&c->strings, buffer_string(&c->string_buffer));
    string_pool_entry *e = (string_pool_entry *)vector_get(&c->strings.entries, s);
    buffer_clear(&c->string_buffer);

    Block *b = get_block(c, f);
    bytecode_put_pushstr(&b->bytecode, e->offset, e->len);
}

static void visit_ident(Compiler *c, Frame *f, node *n) {
    string name = token_string(n->token);
    symbol s = string_pool_intern(&c->idents, name);

    uint8_t id;
    if (!get_strict_local(f, s, &id)) panic("unresolved symbol: " F_STRING, FA_STRING(name));

    Block *b = get_block(c, f);
    bytecode_put_load(&b->bytecode, id);
}

static void visit_binary(Compiler *c, Frame *f, node *n) {
    for (node *child = n->first_child; child; child = child->next_sibling) visit(c, f, child);

    Block *b = get_block(c, f);

    if (string_eq(token_string(n->token), STRING("+"))) buffer_putc(&b->bytecode, OP_ADD);

    // TODO: Other built-ins and user functions
}

static void visit_if(Compiler *c, Frame *f, node *n) {
    Frame then_frame;

    node *cond = n->first_child;
    node *then = cond->next_sibling;
    node *elze = then->next_sibling;

    // prepare a continuation block so that the `elze` block falls through to it
    block_id cont_block = push_block(c);

    block_id then_block = push_block(c);
    frame_init(&then_frame, f, then_block);
    visit(c, &then_frame, then);
    frame_free(&then_frame);
    put_jump(c, f, OP_JUMP, cont_block);

    visit(c, f, cond);
    put_jump(c, f, OP_JUMPIF, then_block);
    visit(c, f, elze);

    f->block = cont_block; // resume in continuation block
}

static void visit_doblk(Compiler *c, Frame *f, node *n) {
    if (node_is_expr(*n->first_child)) {
        visit(c, f, n->first_child);
        return;
    }

    node *lhs, *rhs;
    node *stmt = n->first_child;
    node  cont = {
         .type = N_DOBLK,
         .first_child = stmt->next_sibling,
    };

    switch (stmt->type) {
    case N_ASSIGN:
        lhs = stmt->first_child;
        rhs = lhs->next_sibling;
        visit_pattern(c, f, lhs, rhs, &cont);
        break;
    default: panic("unsupported node: %s", node_type_name(stmt->type));
    }
}

static void visit(Compiler *c, Frame *f, node *n) {
    switch (n->type) {
    case N_INT: visit_int(c, f, n); break;
    case N_STRING: visit_string(c, f, n); break;
    case N_IDENT: visit_ident(c, f, n); break;
    case N_BINARY: visit_binary(c, f, n); break;
    case N_IF: visit_if(c, f, n); break;
    case N_DOBLK: visit_doblk(c, f, n); break;
    default: panic("unsupported node: %s", node_type_name(n->type));
    }
}

static void visit_pident(Compiler *c, Frame *f, node *lhs, node *rhs, node *cont) {
    visit(c, f, rhs);

    symbol  s = string_pool_intern(&c->idents, token_string(lhs->token));
    uint8_t i = get_or_insert_local(f, s);

    Block *b = get_block(c, f);
    bytecode_put_store(&b->bytecode, i);

    visit(c, f, cont);
}

static void visit_pattern(Compiler *c, Frame *f, node *lhs, node *rhs, node *cont) {
    switch (lhs->type) {
    case N_PIDENT: visit_pident(c, f, lhs, rhs, cont); break;
    default: panic("unsupported node: %s", node_type_name(lhs->type));
    }
}

void compiler_visit_program(Compiler *c, node *n) {
    Frame f;
    frame_init(&f, NULL, push_block(c));

    visit(c, &f, n);

    Block *b = get_block(c, &f);
    frame_free(&f);

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

    for (EACH_IN_VECTOR(c->blocks, Block, block)) buffer_puts(b, buffer_string(&block->bytecode));
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
            default: buffer_putc(out, str.data[i]);
            }

            escape = false;
        } else {
            buffer_putc(out, str.data[i]);
        }
    }
}
