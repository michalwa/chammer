#include "compiler.h"

#include <string.h>

#include "bytecode.h"
#include "bytes.h"
#include "utils.h"

/*
 * A logical block of bytecode, either a branch or a function
 */
typedef struct Block Block;
struct Block {
    Block *next;
    Buffer bytecode;
    size_t offset;
};

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
    Block *origin;
    Block *target;
    /*
     * byte offset of instruction address relative to start of `origin`
     */
    size_t addr_offset;
    /*
     * byte offset of the jump instruction relative to start of `origin`
     */
    size_t origin_offset;
} jump;

/*
 * Function metadata, later translated into `func_meta` with a resolved address
 */
typedef struct {
    Block *start;
    size_t locals;
    size_t args;
} func;

static void block_init(Block *b) {
    buffer_init(&b->bytecode);
    b->offset = 0; // offset is only calcuated in `compiler_write_program`
}

static void block_free(Block *b) {
    buffer_free(&b->bytecode);
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

void compiler_init(Compiler *c) {
    buffer_init(&c->string_buffer);
    string_pool_init(&c->strings);
    string_pool_init(&c->idents);
    stack_init(&c->blocks, Block);
    vector_init(&c->jumps, jump);
    vector_init(&c->funcs, func);
}

void compiler_free(Compiler *c) {
    for (stack_iter i = stack_iter_begin(&c->blocks); stack_iter_next(&i);)
        block_free((Block *)i.item);

    buffer_free(&c->string_buffer);
    string_pool_free(&c->strings);
    string_pool_free(&c->idents);
    stack_free(&c->blocks);
    vector_free(&c->jumps);
    vector_free(&c->funcs);
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
    return CHECKED_U8(s->locals.len - 1);
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

/*
 * Finds the symbol in any current or outer scope and propagates captures
 * down through the chain
 */
static bool scope_resolve_symbol(Scope *s, symbol name, uint8_t *id) {
    if (scope_get_local(s, name, id)) return true;

    // The root scope is allowed to store captures for unresolved symbols.
    // These are then treated as external symbols that should be imported at
    // runtime.
    if (!s->outer || scope_resolve_symbol(s->outer, name, NULL)) {
        scope_put_capture(s, name);
        uint8_t local_id = scope_put_local(s, name);
        if (id) *id = local_id;
        return true;
    }

    return false;
}

static void scope_debug_print(Compiler *c, Scope *s, FILE *f) {
    fprintf(f, "scope stack, inner-most first:\n");

    for (size_t i = 0; s; s = s->outer, i++) {
        fprintf(f, "#%zu\n  locals:  ", i);

        for (size_t j = 0; j < s->locals.len; j++) {
            symbol *sym = (symbol *)vector_get(&s->locals, j);
            fprintf(f, " " F_STRING, FA_STRING(string_pool_get(&c->idents, *sym)));
        }

        fprintf(f, "\n  captures:");

        for (size_t j = 0; j < s->captures.len; j++) {
            symbol *sym = (symbol *)vector_get(&s->captures, j);
            fprintf(f, " " F_STRING, FA_STRING(string_pool_get(&c->idents, *sym)));
        }

        fprintf(f, "\n");
    }
}

static inline Block *insert_block_after(Compiler *c, Block *prev) {
    Block *b = (Block *)stack_push_zeroed(&c->blocks);
    block_init(b);
    b->next = prev->next;
    prev->next = b;
    return b;
}

static uint32_t push_func(Compiler *c, Block *start, Scope *inner_scope, size_t args) {
    func *f = (func *)vector_push(&c->funcs);
    f->start = start;
    f->locals = inner_scope->locals.len;
    f->args = args;
    return CHECKED_U32(c->funcs.len - 1);
}

static void put_jump(Compiler *c, opcode op, Block *origin, Block *target) {
    jump *j = (jump *)vector_push(&c->jumps);
    j->origin = origin;
    j->target = target;
    j->origin_offset = origin->bytecode.len;
    bytecode_put_jump(&origin->bytecode, op, &j->addr_offset);
}

/*
 * Stores captures as locals (in reverse order, because they are pushed onto the
 * stack in the original order)
 */
static void put_store_captures(Compiler *c, Block *b, Scope *inner_scope) {
    for (int i = inner_scope->captures.len - 1; i >= 0; i--) {
        symbol *capture = (symbol *)vector_get(&inner_scope->captures, i);
        uint8_t inner_local;

        if (!scope_get_local(inner_scope, *capture, &inner_local)) {
            scope_debug_print(c, inner_scope, stderr);
            panic("unused capture: " F_STRING, FA_STRING(string_pool_get(&c->idents, *capture)));
        }

        bytecode_put_store(&b->bytecode, inner_local);
    }
}

/*
 * Loads captures from outer locals
 */
static void put_load_captures(Compiler *c, Block *b, Scope *inner_scope, Scope *outer_scope) {
    for (size_t i = 0; i < inner_scope->captures.len; i++) {
        symbol *capture = (symbol *)vector_get(&inner_scope->captures, i);
        uint8_t outer_local;

        if (!scope_get_local(outer_scope, *capture, &outer_local)) {
            scope_debug_print(c, outer_scope, stderr);
            panic(
                "unresolved capture: " F_STRING, FA_STRING(string_pool_get(&c->idents, *capture))
            );
        }

        bytecode_put_load(&b->bytecode, outer_local);
    }
}

/*
 * Loads external symbols identified by captures in the given scope
 */
static void put_load_externs(Compiler *c, Block *b, Scope *scope) {
    for (size_t i = 0; i < scope->captures.len; i++) {
        symbol *sym = (symbol *)vector_get(&scope->captures, i);
        string  name = string_pool_get(&c->idents, *sym);
        uint8_t local;

        if (!scope_get_local(scope, *sym, &local)) {
            scope_debug_print(c, scope, stderr);
            panic("missing local for extern: " F_STRING, FA_STRING(name));
        }

        symbol             name_sym = string_pool_intern(&c->strings, name);
        string_pool_entry *name_entry =
            (string_pool_entry *)vector_get(&c->strings.entries, name_sym);

        bytecode_put_pushstr(&b->bytecode, name_entry->offset, name_entry->len);
        buffer_putc(&b->bytecode, (char)OP_LOADEXT);
        bytecode_put_store(&b->bytecode, local);
    }
}

static void visit_expr(Compiler *, Block **, Scope *, node *);
static void visit_pattern(Compiler *, Block **, Block **fail, Scope *, node *lhs, node *rhs);

static void visit_ident(Compiler *c, Block **b, Scope *s, node *n) {
    string name = token_string(n->token);
    symbol sym = string_pool_intern(&c->idents, name);

    uint8_t id;
    if (!scope_resolve_symbol(s, sym, &id)) {
        scope_debug_print(c, s, stderr);
        panic("unresolved symbol: " F_STRING, FA_STRING(name));
    }

    bytecode_put_load(&(*b)->bytecode, id);
}

static void visit_string(Compiler *c, Block **b, Scope *s, node *n) {
    (void)s;

    compile_string(token_string(n->token), &c->string_buffer);
    symbol             sym = string_pool_intern(&c->strings, buffer_string(&c->string_buffer));
    string_pool_entry *e = (string_pool_entry *)vector_get(&c->strings.entries, sym);

    buffer_clear(&c->string_buffer);
    bytecode_put_pushstr(&(*b)->bytecode, e->offset, e->len);
}

static void visit_int(Compiler *c, Block **b, Scope *s, node *n) {
    (void)c;
    (void)s;
    bytecode_put_pushint(&(*b)->bytecode, compile_int(token_string(n->token)));
}

static void visit_tuple(Compiler *c, Block **b, Scope *s, node *n) {
    uint8_t len = 0;
    for (node *child = n->first_child; child; child = child->next_sibling) {
        visit_expr(c, b, s, child);
        len++;
    }

    bytecode_put_maketuple(&(*b)->bytecode, len);
}

static void visit_list(Compiler *c, Block **b, Scope *s, node *n) {
    size_t  parts = 1;
    uint8_t part_len = 0;

    if (!n->first_child) {
        bytecode_put_makelist(&(*b)->bytecode, 0);
        return;
    }

    for (node *child = n->first_child; child; child = child->next_sibling) {
        if (child->type == N_SPREAD) {
            visit_expr(c, b, s, child->first_child);
            parts++;
        } else {
            visit_expr(c, b, s, child);
            part_len++;
        }

        if (!child->next_sibling || child->next_sibling->type == N_SPREAD) {
            if (child->type != N_SPREAD) {
                bytecode_put_makelist(&(*b)->bytecode, part_len);
                parts++;
                part_len = 0;
            }
        }

        if (parts > 2) {
            buffer_putc(&(*b)->bytecode, OP_CONCAT);
            parts--;
        }
    }
}

static void visit_unary(Compiler *c, Block **b, Scope *s, node *n) {
    if (string_eq(token_string(n->token), STRING("-")) && n->first_child->type == N_INT) {
        int64_t value = compile_int(token_string(n->first_child->token));
        bytecode_put_pushint(&(*b)->bytecode, -value);
        return;
    }

    visit_expr(c, b, s, n->first_child);
    visit_ident(c, b, s, n);
    bytecode_put_callcls(&(*b)->bytecode, 2);
}

static void visit_binary(Compiler *c, Block **b, Scope *s, node *n) {
    for (node *child = n->first_child; child; child = child->next_sibling)
        visit_expr(c, b, s, child);

    // TODO: Remove special case for (+), this is for testing purposes only
    if (string_eq(token_string(n->token), STRING("+"))) {
        buffer_putc(&(*b)->bytecode, OP_ADD);
        return;
    }

    visit_ident(c, b, s, n);
    bytecode_put_callcls(&(*b)->bytecode, 2);
}

static void visit_apply(Compiler *c, Block **b, Scope *s, node *n) {
    int items = 0;
    for (node *child = n->first_child; child; child = child->next_sibling) items++;

    for (int i = items - 1; i >= 0; i--) {
        node *nth = n->first_child;
        for (int j = 0; j < i; j++) nth = nth->next_sibling;

        visit_expr(c, b, s, nth);
    }

    bytecode_put_callcls(&(*b)->bytecode, (uint8_t)(items - 1));
}

static void visit_if(Compiler *c, Block **b, Scope *s, node *n) {
    node *cond = n->first_child;
    node *then = cond->next_sibling;
    node *elze = then->next_sibling;

    Block *else_block = insert_block_after(c, *b);

    visit_expr(c, b, s, cond);
    put_jump(c, OP_JUMPIFN, *b, else_block);
    visit_expr(c, b, s, then);

    Block *cont_block = insert_block_after(c, *b);

    visit_expr(c, &else_block, s, elze);
    put_jump(c, OP_JUMP, else_block, cont_block);

    *b = cont_block; // resume in continuation block
}

static void visit_match(Compiler *c, Block **b, Scope *s, node *n) {
    node *scrutinee = n->first_child;
    visit_expr(c, b, s, scrutinee);

    Block *cont_block = insert_block_after(c, *b);

    for (node *child = scrutinee->next_sibling; child;) {
        node *rhs = child->next_sibling;

        if (rhs) {
            Scope inner_scope;
            scope_init(&inner_scope, s);

            buffer_putc(&(*b)->bytecode, OP_DUP);

            Block *fail = NULL;
            Block *prelude = insert_block_after(c, cont_block);
            Block *case_body = insert_block_after(c, prelude);

            visit_pattern(c, &case_body, &fail, &inner_scope, child, NULL);
            visit_expr(c, &case_body, &inner_scope, rhs);
            buffer_putc(&case_body->bytecode, OP_PUSHTRUE); // `true` means success
            buffer_putc(&case_body->bytecode, OP_RETURN);

            put_store_captures(c, prelude, &inner_scope);
            put_load_captures(c, *b, &inner_scope, s);
            uint32_t fnindex = push_func(c, prelude, &inner_scope, 1);
            bytecode_put_call(&(*b)->bytecode, fnindex);

            put_jump(c, OP_JUMPIF, *b, cont_block);
            buffer_putc(&(*b)->bytecode, OP_POP); // pop `false`

            scope_free(&inner_scope);

            if (fail) {
                buffer_putc(&fail->bytecode, OP_PUSHFALSE); // `false` means error
                buffer_putc(&fail->bytecode, OP_RETURN);
            } else {
                // TODO: emit warning
                // infallible pattern, no point compiling following cases
                break;
            }

            child = rhs->next_sibling;
        } else {
            buffer_putc(&(*b)->bytecode, OP_POP);
            visit_expr(c, b, s, child);
            put_jump(c, OP_JUMP, *b, cont_block);
            break;
        }
    }

    buffer_putc(&(*b)->bytecode, (char)OP_HALT); // TODO: raise error

    *b = cont_block;
}

/*
 * Universal implementation for `N_LAMBDA` and `N_PAPPLY`
 *
 * `arg0` and its siblings will be iterated until `body` or `NULL`
 *
 * `self` can point to the name of a recursive definition, in which case the
 * compiled function will be equipped with a self-reference
 */
static void visit_function(Compiler *c, Block **b, Scope *s, node *arg0, node *body, symbol *self) {
    Scope inner_scope;
    scope_init(&inner_scope, s);

    Block *prelude = insert_block_after(c, *b);
    Block *body_block = insert_block_after(c, prelude);
    Block *fail = NULL;

    uint8_t args = 0;

    if (self) {
        uint8_t self_local = scope_put_local(&inner_scope, *self);

        buffer_putc(&body_block->bytecode, OP_DUP);
        bytecode_put_callcls(&body_block->bytecode, 1);
        bytecode_put_store(&body_block->bytecode, self_local);

        args++;
    }

    for (node *arg = arg0; arg && arg != body; arg = arg->next_sibling) {
        visit_pattern(c, &body_block, &fail, &inner_scope, arg, NULL);
        args++;
    }

    visit_expr(c, &body_block, &inner_scope, body);
    buffer_putc(&body_block->bytecode, OP_RETURN);

    if (fail) buffer_putc(&fail->bytecode, (char)OP_HALT); // TODO: raise error

    put_store_captures(c, prelude, &inner_scope);
    put_load_captures(c, *b, &inner_scope, s);
    uint32_t fnindex = push_func(c, prelude, &inner_scope, args);
    bytecode_put_makecls(&(*b)->bytecode, fnindex, inner_scope.captures.len);

    scope_free(&inner_scope);

    if (self) {
        buffer_putc(&(*b)->bytecode, OP_DUP);
        bytecode_put_callcls(&(*b)->bytecode, 1);
    }
}

static void visit_lambda(Compiler *c, Block **b, Scope *s, node *n) {
    node *body = n->first_child;
    while (body->next_sibling) body = body->next_sibling;

    visit_function(c, b, s, n->first_child, body, NULL);
}

static void visit_block(Compiler *c, Block **b, Scope *s, node *n) {
    Scope inner_scope;
    scope_init(&inner_scope, s);

    Block *prelude = insert_block_after(c, *b);
    Block *body = insert_block_after(c, prelude);
    Block *fail = NULL;

    for (node *child = n->first_child; child; child = child->next_sibling) {
        if (!child->next_sibling) {
            visit_expr(c, &body, &inner_scope, child);
            break;
        }

        node *lhs;

        switch (child->type) {
        case N_ASSIGN:
            lhs = child->first_child;
            visit_pattern(c, &body, &fail, &inner_scope, lhs, lhs->next_sibling);
            break;
        default: panic("unsupported node: %s", node_type_name(child->type));
        }
    }

    buffer_putc(&body->bytecode, OP_RETURN);

    put_store_captures(c, prelude, &inner_scope);
    put_load_captures(c, *b, &inner_scope, s);
    uint32_t fnindex = push_func(c, prelude, &inner_scope, 0);
    bytecode_put_call(&(*b)->bytecode, fnindex);

    if (fail) buffer_putc(&fail->bytecode, (char)OP_HALT); // TODO: raise error

    scope_free(&inner_scope);
}

static void visit_expr(Compiler *c, Block **b, Scope *s, node *n) {
    switch (n->type) {
    case N_IDENT: visit_ident(c, b, s, n); break;
    case N_STRING: visit_string(c, b, s, n); break;
    case N_INT: visit_int(c, b, s, n); break;
    case N_TUPLE: visit_tuple(c, b, s, n); break;
    case N_LIST: visit_list(c, b, s, n); break;
    case N_UNARY: visit_unary(c, b, s, n); break;
    case N_BINARY: visit_binary(c, b, s, n); break;
    case N_APPLY: visit_apply(c, b, s, n); break;
    case N_IF: visit_if(c, b, s, n); break;
    case N_MATCH: visit_match(c, b, s, n); break;
    case N_LAMBDA: visit_lambda(c, b, s, n); break;
    // TODO: Remove do-blocks?
    case N_DOBLK: visit_block(c, b, s, n); break;
    case N_BLOCK: visit_block(c, b, s, n); break;
    default: panic("unsupported node: %s", node_type_name(n->type));
    }
}

static void visit_pident(Compiler *c, Block **b, Block **fail, Scope *s, node *lhs, node *rhs) {
    (void)fail;

    if (rhs) visit_expr(c, b, s, rhs);

    symbol  sym = string_pool_intern(&c->idents, token_string(lhs->token));
    uint8_t i = scope_get_or_put_local(s, sym);

    bytecode_put_store(&(*b)->bytecode, i);
}

static void visit_papply(Compiler *c, Block **b, Block **fail, Scope *s, node *lhs, node *rhs) {
    (void)fail;

    symbol  sym = string_pool_intern(&c->idents, token_string(lhs->token));
    uint8_t i = scope_get_or_put_local(s, sym);

    visit_function(c, b, s, lhs->first_child, rhs, (lhs->flags & NF_REC) ? &sym : NULL);
    bytecode_put_store(&(*b)->bytecode, i);
}

static void visit_ptuple(Compiler *c, Block **b, Block **fail, Scope *s, node *lhs, node *rhs) {
    if (rhs) visit_expr(c, b, s, rhs);

    uint8_t len = 0;
    for (node *child = lhs->first_child; child; child = child->next_sibling) len++;

    if (!*fail) *fail = insert_block_after(c, *b);

    bytecode_put_istuple(&(*b)->bytecode, len);
    put_jump(c, OP_JUMPIFN, *b, *fail);

    uint8_t index = 0;
    for (node *child = lhs->first_child; child; child = child->next_sibling) {
        bytecode_put_tupleget(&(*b)->bytecode, index);
        visit_pattern(c, b, fail, s, child, NULL);
        index++;
    }

    buffer_putc(&(*b)->bytecode, OP_POP);
}

static void visit_plist(Compiler *c, Block **b, Block **fail, Scope *s, node *lhs, node *rhs) {
    if (rhs) visit_expr(c, b, s, rhs);

    if (!*fail) *fail = insert_block_after(c, *b);

    for (node *child = lhs->first_child; child; child = child->next_sibling) {
        if (child->type == N_PLTAIL) {
            if (child->flags & NF_NAMED) {
                symbol  sym = string_pool_intern(&c->idents, token_string(child->token));
                uint8_t i = scope_get_or_put_local(s, sym);
                bytecode_put_store(&(*b)->bytecode, i);
            } else {
                buffer_putc(&(*b)->bytecode, OP_POP);
            }
            return;
        }

        buffer_putc(&(*b)->bytecode, OP_ISCONS);
        put_jump(c, OP_JUMPIFN, *b, *fail);
        buffer_putc(&(*b)->bytecode, OP_UNCONS);
        visit_pattern(c, b, fail, s, child, NULL);
    }

    buffer_putc(&(*b)->bytecode, OP_ISNIL);
    put_jump(c, OP_JUMPIFN, *b, *fail);
    buffer_putc(&(*b)->bytecode, OP_POP);
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
static void visit_pattern(Compiler *c, Block **b, Block **fail, Scope *s, node *lhs, node *rhs) {
    switch (lhs->type) {
    case N_PIDENT: visit_pident(c, b, fail, s, lhs, rhs); break;
    case N_PAPPLY: visit_papply(c, b, fail, s, lhs, rhs); break;
    case N_PTUPLE: visit_ptuple(c, b, fail, s, lhs, rhs); break;
    case N_PLIST: visit_plist(c, b, fail, s, lhs, rhs); break;
    default: panic("unsupported node: %s", node_type_name(lhs->type));
    }
}

void compiler_visit_program(Compiler *c, node *n) {
    Scope global_scope;
    scope_init(&global_scope, NULL);

    Block *prelude = (Block *)stack_push_zeroed(&c->blocks);
    block_init(prelude);

    Block *block = insert_block_after(c, prelude);
    visit_expr(c, &block, &global_scope, n);
    buffer_putc(&block->bytecode, (char)OP_HALT);
    put_load_externs(c, prelude, &global_scope);

    scope_free(&global_scope);
}

void compiler_write_program(Compiler *c, Buffer *b) {
    buffer_puts(b, STRING(MAGIC_HAMMER));
    buffer_put_u16be(b, BYTECODE_VERSION);
    buffer_put_u32be(b, CHECKED_U32(c->strings.buffer.len));
    buffer_puts(b, buffer_string(&c->strings.buffer));
    buffer_put_u32be(b, CHECKED_U32(c->funcs.len));

    Block *prelude_block = (Block *)stack_get(&c->blocks, 0);

    size_t block_offset = 0;
    for (Block *block = prelude_block; block; block = block->next) {
        block->offset = block_offset;
        block_offset += block->bytecode.len;
    }

    for (EACH_IN_VECTOR(c->jumps, jump, j)) {
        uint8_t *addr = (uint8_t *)j->origin->bytecode.data + j->addr_offset;
        long     jump_rel_addr =
            (long)j->target->offset - (long)j->origin->offset - (long)j->origin_offset;
        write_i16be(&addr, CHECKED_I16(jump_rel_addr));
    }

    for (EACH_IN_VECTOR(c->funcs, func, f)) {
        bytecode_put_func_meta(
            b,
            (func_meta){
                .addr = f->start->offset,
                .locals = CHECKED_U8(f->locals),
                .args = CHECKED_U8(f->args),
            }
        );
    }

    for (Block *block = prelude_block; block; block = block->next)
        buffer_puts(b, buffer_string(&block->bytecode));
}

int64_t compile_int(string str) {
    int64_t value = 0;
    for (size_t i = 0; i < str.len; i++) value = (value * 10) + (str.data[i] - '0');
    return value;
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
