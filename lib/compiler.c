#include <string.h>

#include "bytecode.h"
#include "compiler.h"
#include "utils.h"

typedef struct {
    Buffer bytecode;
} Proc;

typedef struct {
    size_t proc_index;
} frame;

typedef struct {
    size_t proc_index;
    size_t addr_offset;
    size_t target_proc_index;
} jump;

static void proc_init(Proc *p) {
    buffer_init(&p->bytecode);
}

static inline void compiler_begin_proc(Compiler *c) {
    proc_init((Proc *)stack_push(&c->procs));
    frame *f = (frame *)stack_push(&c->frames);
    f->proc_index = c->procs.size - 1;
}

static inline void compiler_end_proc(Compiler *c) {
    stack_pop(&c->frames);
}

static inline Proc *compiler_current_proc(Compiler *c) {
    frame *current_frame = (frame *)stack_top(&c->frames);
    return (Proc *)stack_get(&c->procs, current_frame->proc_index);
}

static void proc_free(Proc *p) {
    buffer_free(&p->bytecode);
}

void compiler_init(Compiler *c) {
    buffer_init(&c->string_buffer);
    stack_init(&c->procs, Proc);
    stack_init(&c->frames, frame);
    stack_init(&c->jumps, jump);

    compiler_begin_proc(c);
}

void compiler_free(Compiler *c) {
    buffer_free(&c->string_buffer);

    for (stack_iter i = stack_iter_begin(&c->procs); stack_iter_next(&i);)
        proc_free((Proc *)i.item);

    stack_free(&c->procs);
    stack_free(&c->frames);
    stack_free(&c->jumps);
}

static void compiler_visit_int(Compiler *c, node *n) {
    uint64_t value = 0;
    for (size_t i = 0; i < n->token.len; i++)
        value = (value * 10) + (n->token.str[i] - '0');

    Proc *proc = compiler_current_proc(c);
    buffer_putc(&proc->bytecode, OP_PUSHINT);
    buffer_write_u64be(&proc->bytecode, value);
}

static void compiler_visit_string(Compiler *c, node *n) {
    // TODO: String interning

    size_t offset, len;
    compile_string(token_string(n->token), &c->string_buffer, &offset, &len);

    Proc *proc = compiler_current_proc(c);
    buffer_putc(&proc->bytecode, OP_PUSHSTR);
    buffer_write_u32be(&proc->bytecode, (uint32_t)offset);
    buffer_write_u32be(&proc->bytecode, (uint32_t)len);
}

static void compiler_visit_binary(Compiler *c, node *n) {
    for (node *child = n->first_child; child; child = child->next_sibling)
        compiler_visit(c, child);

    Proc *proc = compiler_current_proc(c);

    if (string_eq(token_string(n->token), STRING("+")))
        buffer_putc(&proc->bytecode, OP_ADD);

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
    case N_INT: compiler_visit_int(c, n); break;
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
    // buffer_puts(b, c->bytecode.data, c->bytecode.len);
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
