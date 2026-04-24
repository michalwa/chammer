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

#define BLOCK_NULL ((block_id) - 1)

typedef struct Scope Scope;
struct Scope {
    Scope *outer;
    Vector locals;
    Vector captures;
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

static void block_init(Block *b) {
    buffer_init(&b->bytecode);
    b->offset = 0; // offset is only calcuated in `compiler_write_program`
}

static void block_free(Block *b) {
    buffer_free(&b->bytecode);
}

void compiler_init(Compiler *c) {
    buffer_init(&c->string_buffer);
    string_pool_init(&c->strings);
    string_pool_init(&c->idents);
    vector_init(&c->blocks, Block);
    vector_init(&c->jumps, jump);
}

void compiler_free(Compiler *c) {
    for (EACH_IN_VECTOR(c->blocks, Block, block)) block_free(block);

    buffer_free(&c->string_buffer);
    string_pool_free(&c->strings);
    string_pool_free(&c->idents);
    vector_free(&c->blocks);
    vector_free(&c->jumps);
}

static void scope_init(Scope *s, Scope *outer) {
    s->outer = outer;
    vector_init(&s->locals, symbol);
    vector_init(&s->captures, symbol);
}

static void scope_free(Scope *s) {
    vector_free(&s->locals);
    vector_free(&s->captures);
}

/*
 * Resolves the name to a local ID without searching previous frames
 */
static bool scope_get_local(Scope *s, symbol name, uint8_t *id) {
    for (uint8_t i = 0; i < (uint8_t)s->locals.len; i++) {
        symbol *sym = (symbol *)vector_get(&s->locals, i);
        if (*sym == name) {
            if (id) *id = i;
            return true;
        }
    }

    return false;
}

static uint8_t scope_put_local(Scope *s, symbol name) {
    *(symbol *)vector_push(&s->locals) = name;
    return (uint8_t)(s->locals.len - 1);
}

static inline uint8_t scope_get_or_put_local(Scope *s, symbol name) {
    uint8_t id;
    if (scope_get_local(s, name, &id)) return id;
    return scope_put_local(s, name);
}

static void scope_put_capture(Scope *s, symbol name) {
    for (size_t i = 0; i < s->captures.len; i++) {
        symbol *sym = (symbol *)vector_get(&s->captures, i);
        if (*sym == name) return;
    }

    *(symbol *)vector_push(&s->captures) = name;
}

static bool scope_resolve_symbol(Scope *s, symbol name, uint8_t *id) {
    if (scope_get_local(s, name, id)) return true;

    if (s->outer && scope_resolve_symbol(s->outer, name, NULL)) {
        scope_put_capture(s, name);
        *id = scope_put_local(s, name);
        return true;
    }

    return false;
}

static inline block_id push_block(Compiler *c) {
    block_init((Block *)vector_push(&c->blocks));
    return c->blocks.len - 1;
}

static inline Block *get_block(Compiler *c, block_id bid) {
    debug_assert(bid != BLOCK_NULL);
    return (Block *)vector_get(&c->blocks, bid);
}

static void put_jump(Compiler *c, opcode op, block_id from_block, block_id to_block) {
    jump *j = (jump *)vector_push(&c->jumps);
    j->from_block = from_block;
    j->to_block = to_block;

    Block *b = get_block(c, from_block);
    bytecode_put_jump(&b->bytecode, op, &j->addr_offset);
}

static void put_call(Compiler *c, block_id from_block, block_id to_block, Scope *inner_scope) {
    jump *j = (jump *)vector_push(&c->jumps);
    j->from_block = from_block;
    j->to_block = to_block;

    Block *b = get_block(c, from_block);
    bytecode_put_call(&b->bytecode, (uint8_t)inner_scope->locals.len, &j->addr_offset);
}

static void put_makecls(Compiler *c, block_id bid, uint8_t captures, uint8_t args, block_id body) {
    jump *j = (jump *)vector_push(&c->jumps);
    j->from_block = bid;
    j->to_block = body;

    Block *b = get_block(c, bid);
    bytecode_put_makecls(&b->bytecode, captures, args, &j->addr_offset);
}

static void put_store_captures(Compiler *c, block_id bid, Scope *inner_scope) {
    Block *b = get_block(c, bid);

    // Store captures as locals (in reverse order, because they are pushed onto
    // the stack in the original order)
    for (int i = inner_scope->captures.len - 1; i >= 0; i--) {
        symbol *capture = (symbol *)vector_get(&inner_scope->captures, i);
        uint8_t inner_local;

        if (!scope_get_local(inner_scope, *capture, &inner_local))
            panic("unused capture: " F_STRING, FA_STRING(string_pool_get(&c->idents, *capture)));

        bytecode_put_store(&b->bytecode, inner_local);
    }
}

static void put_load_captures(Compiler *c, block_id bid, Scope *inner_scope, Scope *outer_scope) {
    Block *b = get_block(c, bid);

    // Load captures from outer locals
    for (size_t i = 0; i < inner_scope->captures.len; i++) {
        symbol *capture = (symbol *)vector_get(&inner_scope->captures, i);
        uint8_t outer_local;

        if (!scope_get_local(outer_scope, *capture, &outer_local))
            panic(
                "unresolved capture: " F_STRING, FA_STRING(string_pool_get(&c->idents, *capture))
            );

        bytecode_put_load(&b->bytecode, outer_local);
    }
}

static void visit_expr(Compiler *, Scope *, block_id *, node *);
static void visit_pattern(
    Compiler *, Scope *, block_id *bid, node *lhs, node *rhs, block_id *fail_bid
);

static void visit_ident(Compiler *c, Scope *scope, block_id bid, node *n) {
    string name = token_string(n->token);
    symbol sym = string_pool_intern(&c->idents, name);

    uint8_t id;
    if (!scope_resolve_symbol(scope, sym, &id))
        panic("unresolved symbol: " F_STRING, FA_STRING(name));

    Block *b = get_block(c, bid);
    bytecode_put_load(&b->bytecode, id);
}

static void visit_string(Compiler *c, block_id bid, node *n) {
    compile_string(token_string(n->token), &c->string_buffer);
    symbol             s = string_pool_intern(&c->strings, buffer_string(&c->string_buffer));
    string_pool_entry *e = (string_pool_entry *)vector_get(&c->strings.entries, s);
    buffer_clear(&c->string_buffer);

    Block *b = get_block(c, bid);
    bytecode_put_pushstr(&b->bytecode, e->offset, e->len);
}

static void visit_int(Compiler *c, block_id bid, node *n) {
    uint64_t value = 0;
    for (size_t i = 0; i < n->token.len; i++) value = (value * 10) + (n->token.str[i] - '0');

    Block *b = get_block(c, bid);
    bytecode_put_pushint(&b->bytecode, value);
}

static void visit_tuple(Compiler *c, Scope *scope, block_id *bid, node *n) {
    uint8_t len = 0;
    for (node *child = n->first_child; child; child = child->next_sibling) {
        visit_expr(c, scope, bid, child);
        len++;
    }

    Block *b = get_block(c, *bid);
    bytecode_put_maketuple(&b->bytecode, len);
}

static void visit_list(Compiler *c, Scope *scope, block_id *bid, node *n) {
    uint8_t len = 0;
    for (node *child = n->first_child; child; child = child->next_sibling) {
        visit_expr(c, scope, bid, child);
        len++;
    }

    Block *b = get_block(c, *bid);
    bytecode_put_makelist(&b->bytecode, len);
}

static void visit_binary(Compiler *c, Scope *scope, block_id *bid, node *n) {
    for (node *child = n->first_child; child; child = child->next_sibling)
        visit_expr(c, scope, bid, child);

    Block *b = get_block(c, *bid);

    if (string_eq(token_string(n->token), STRING("+"))) buffer_putc(&b->bytecode, OP_ADD);

    // TODO: Other built-ins and user functions
}

static void visit_apply(Compiler *c, Scope *scope, block_id *bid, node *n) {
    int items = 0;
    for (node *child = n->first_child; child; child = child->next_sibling) items++;

    for (int i = items - 1; i >= 0; i--) {
        node *nth = n->first_child;
        for (int j = 0; j < i; j++) nth = nth->next_sibling;

        visit_expr(c, scope, bid, nth);
    }

    Block *b = get_block(c, *bid);
    bytecode_put_callcls(&b->bytecode, (uint8_t)(items - 1));
}

static void visit_if(Compiler *c, Scope *scope, block_id *bid, node *n) {
    node *cond = n->first_child;
    node *then = cond->next_sibling;
    node *elze = then->next_sibling;

    // prepare a continuation block so that the `then` block falls through to it
    block_id cont_block = push_block(c);

    block_id elze_block = push_block(c);
    visit_expr(c, scope, &elze_block, elze);
    put_jump(c, OP_JUMP, *bid, cont_block);

    visit_expr(c, scope, bid, cond);
    put_jump(c, OP_JUMPIFN, *bid, elze_block);
    visit_expr(c, scope, bid, then);

    *bid = cont_block; // resume in continuation block
}

static void visit_match(Compiler *c, Scope *scope, block_id *bid, node *n) {
    Block *b;

    node *scrutinee = n->first_child;
    visit_expr(c, scope, bid, scrutinee);

    block_id cont_block = push_block(c);

    for (node *child = scrutinee->next_sibling; child;) {
        node *rhs = child->next_sibling;

        if (rhs) {
            Scope inner_scope;
            scope_init(&inner_scope, scope);

            b = get_block(c, *bid);
            buffer_putc(&b->bytecode, OP_DUP);

            block_id fail_bid = BLOCK_NULL;
            block_id prelude_bid = push_block(c);
            block_id case_body_bid = push_block(c);

            visit_pattern(c, &inner_scope, &case_body_bid, child, NULL, &fail_bid);
            visit_expr(c, &inner_scope, &case_body_bid, rhs);
            b = get_block(c, case_body_bid);
            buffer_putc(&b->bytecode, OP_PUSHTRUE); // `true` means success
            buffer_putc(&b->bytecode, OP_RETURN);

            put_store_captures(c, prelude_bid, &inner_scope);
            put_load_captures(c, *bid, &inner_scope, scope);
            put_call(c, *bid, prelude_bid, &inner_scope);
            put_jump(c, OP_JUMPIF, *bid, cont_block);
            buffer_putc(&b->bytecode, OP_POP); // pop `false`

            scope_free(&inner_scope);

            if (fail_bid != BLOCK_NULL) {
                b = get_block(c, fail_bid);
                buffer_putc(&b->bytecode, OP_PUSHFALSE); // `false` means error
                buffer_putc(&b->bytecode, OP_RETURN);
            } else {
                // TODO: emit warning
                // infallible pattern, no point compiling following cases
                break;
            }

            child = rhs->next_sibling;
        } else {
            b = get_block(c, *bid);
            buffer_putc(&b->bytecode, OP_POP);
            visit_expr(c, scope, bid, child);
            put_jump(c, OP_JUMP, *bid, cont_block);
            break;
        }
    }

    b = get_block(c, *bid);
    buffer_putc(&b->bytecode, (char)OP_HALT); // TODO: raise error

    *bid = cont_block;
}

/*
 * Universal implementation for N_LAMBDA and N_PAPPLY
 */
static void visit_function(Compiler *c, Scope *scope, block_id bid, node *first_arg, node *body) {
    Scope inner_scope;
    scope_init(&inner_scope, scope);

    block_id prelude_bid = push_block(c);
    block_id body_bid = push_block(c);
    block_id fail_bid = BLOCK_NULL;

    uint8_t args = 0;
    for (node *arg = first_arg; arg && arg != body; arg = arg->next_sibling) {
        visit_pattern(c, &inner_scope, &body_bid, arg, NULL, &fail_bid);
        args++;
    }

    visit_expr(c, &inner_scope, &body_bid, body);
    Block *b = get_block(c, body_bid);
    buffer_putc(&b->bytecode, OP_RETURN);

    if (fail_bid != BLOCK_NULL) {
        Block *fail = get_block(c, fail_bid);
        buffer_putc(&fail->bytecode, (char)OP_HALT); // TODO: raise error
    }

    put_store_captures(c, prelude_bid, &inner_scope);
    put_load_captures(c, bid, &inner_scope, scope);
    put_makecls(c, bid, inner_scope.captures.len, args, prelude_bid);

    scope_free(&inner_scope);
}

static void visit_lambda(Compiler *c, Scope *scope, block_id bid, node *n) {
    node *body = n->first_child;
    while (body->next_sibling) body = body->next_sibling;

    visit_function(c, scope, bid, n->first_child, body);
}

static void visit_doblk(Compiler *c, Scope *scope, block_id *bid, node *n) {
    block_id fail_bid = BLOCK_NULL;

    for (node *child = n->first_child; child; child = child->next_sibling) {
        if (!child->next_sibling) {
            visit_expr(c, scope, bid, child);
            break;
        }

        node *lhs;

        switch (child->type) {
        case N_ASSIGN:
            lhs = child->first_child;
            visit_pattern(c, scope, bid, lhs, lhs->next_sibling, &fail_bid);
            break;
        default: panic("unsupported node: %s", node_type_name(child->type));
        }
    }

    if (fail_bid != BLOCK_NULL) {
        Block *b = get_block(c, fail_bid);
        buffer_putc(&b->bytecode, (char)OP_HALT); // TODO: raise error
    }
}

static void visit_expr(Compiler *c, Scope *scope, block_id *bid, node *n) {
    switch (n->type) {
    case N_IDENT: visit_ident(c, scope, *bid, n); break;
    case N_STRING: visit_string(c, *bid, n); break;
    case N_INT: visit_int(c, *bid, n); break;
    case N_TUPLE: visit_tuple(c, scope, bid, n); break;
    case N_LIST: visit_list(c, scope, bid, n); break;
    case N_BINARY: visit_binary(c, scope, bid, n); break;
    case N_APPLY: visit_apply(c, scope, bid, n); break;
    case N_IF: visit_if(c, scope, bid, n); break;
    case N_MATCH: visit_match(c, scope, bid, n); break;
    case N_LAMBDA: visit_lambda(c, scope, *bid, n); break;
    case N_DOBLK: visit_doblk(c, scope, bid, n); break;
    default: panic("unsupported node: %s", node_type_name(n->type));
    }
}

static void visit_pident(Compiler *c, Scope *scope, block_id bid, node *lhs) {
    symbol  sym = string_pool_intern(&c->idents, token_string(lhs->token));
    uint8_t i = scope_get_or_put_local(scope, sym);

    Block *b = get_block(c, bid);
    bytecode_put_store(&b->bytecode, i);
}

static void visit_papply(Compiler *c, Scope *scope, block_id bid, node *lhs, node *rhs) {
    symbol  sym = string_pool_intern(&c->idents, token_string(lhs->token));
    uint8_t i = scope_get_or_put_local(scope, sym);

    visit_function(c, scope, bid, lhs->first_child, rhs);

    Block *b = get_block(c, bid);
    bytecode_put_store(&b->bytecode, i);
}

static void visit_ptuple(Compiler *c, Scope *scope, block_id *bid, node *lhs, block_id *fail_bid) {
    uint8_t len = 0;
    for (node *child = lhs->first_child; child; child = child->next_sibling) len++;

    if (*fail_bid == BLOCK_NULL) *fail_bid = push_block(c);

    Block *b = get_block(c, *bid);
    bytecode_put_istuple(&b->bytecode, len);
    put_jump(c, OP_JUMPIFN, *bid, *fail_bid);

    uint8_t index = 0;
    for (node *child = lhs->first_child; child; child = child->next_sibling) {
        bytecode_put_tupleget(&b->bytecode, index);
        visit_pattern(c, scope, bid, child, NULL, fail_bid);

        if (!child->next_sibling) {
            b = get_block(c, *bid);
            buffer_putc(&b->bytecode, OP_POP);
        }

        index++;
    }
}

/*
 * For a pattern match `lhs = rhs`:
 *   `lhs` is the pattern,
 *   `rhs` is either the scrutinee/value or `NULL`, in which case the value is
 *     expected on top of the stack,
 *   `bid` is the current block and may be changed,
 *   `fail_bid` can point to a block that will be jumped to in case of failure,
 *     or `-1` in which case a new block will be pushed for this purpose.
 */
static void visit_pattern(
    Compiler *c, Scope *scope, block_id *bid, node *lhs, node *rhs, block_id *fail_bid
) {
    switch (lhs->type) {
    case N_PIDENT:
        if (rhs) visit_expr(c, scope, bid, rhs);
        visit_pident(c, scope, *bid, lhs);
        break;
    case N_PAPPLY: visit_papply(c, scope, *bid, lhs, rhs); break;
    case N_PTUPLE:
        if (rhs) visit_expr(c, scope, bid, rhs);
        visit_ptuple(c, scope, bid, lhs, fail_bid);
        break;
    default: panic("unsupported node: %s", node_type_name(lhs->type));
    }
}

void compiler_visit_program(Compiler *c, node *n) {
    Scope scope;
    scope_init(&scope, NULL);

    block_id prelude_bid = push_block(c);
    block_id bid = push_block(c);

    visit_expr(c, &scope, &bid, n);

    Block *last_block = get_block(c, bid);
    buffer_putc(&last_block->bytecode, (char)OP_HALT);

    put_call(c, prelude_bid, bid, &scope);
    scope_free(&scope);
}

void compiler_write_program(Compiler *c, Buffer *b) {
    buffer_puts(b, STRING(MAGIC_HAMMER));
    bytecode_put_u16be(b, BYTECODE_VERSION);

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
