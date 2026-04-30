#include "machine.h"

#include "bytecode.h"
#include "bytes.h"
#include "utils.h"
#include "value.h"
#include "vector.h"

/*
 * Function call frame header (stack pointer at bottom)
 */
typedef struct {
    // value locals[]; (locals in reverse order)
    uint32_t return_sp;
    uint32_t return_ip;
    uint32_t fnindex;
} frame;

static inline void pop_frame(Machine *);

void machine_init(Machine *m, const program *prog) {
    m->prog = prog;
    buffer_init(&m->fnstack);
    vector_init(&m->opstack, HValue);
    m->ip = prog->bytecode;
}

void machine_free(Machine *m) {
    while (m->fnstack.len > 0) pop_frame(m);

    for (EACH_IN_VECTOR(m->opstack, HValue, value)) hvalue_drop(*value);

    buffer_free(&m->fnstack);
    vector_free(&m->opstack);
}

static inline void push_frame(Machine *m, uint32_t fnindex) {
    func_meta fn;
    program_func_meta(m->prog, fnindex, &fn);

    frame f = {
        .return_ip = m->ip - m->prog->bytecode,
        .return_sp = m->fnstack.len,
        .fnindex = fnindex,
    };
    buffer_alloc(&m->fnstack, sizeof(HValue) * fn.locals + sizeof(frame));
    *(frame *)(m->fnstack.data + m->fnstack.len - sizeof(frame)) = f;

    m->ip = m->prog->bytecode + fn.addr;
}

static inline HValue *get_local(Machine *m, uint8_t i) {
    return (HValue *)(m->fnstack.data + m->fnstack.len - sizeof(frame) - sizeof(HValue) * (i + 1));
}

static inline void pop_frame(Machine *m) {
    frame *f = (frame *)(m->fnstack.data + m->fnstack.len - sizeof(frame));

    for (HValue *local = get_local(m, 0);
         (intptr_t)local - (intptr_t)m->fnstack.data > f->return_sp; local++)
        hvalue_drop(*local);

    m->ip = m->prog->bytecode + f->return_ip;
    buffer_truncate(&m->fnstack, f->return_sp);
}

static inline void load_local(Machine *m, uint8_t i) {
    *(HValue *)vector_push(&m->opstack) = hvalue_ref(*get_local(m, i));
}

static inline void store_local(Machine *m, uint8_t i) {
    debug_assert(vector_pop(&m->opstack, get_local(m, i)));
}

static inline void opstack_dup(Machine *m) {
    *(HValue *)vector_push(&m->opstack) = hvalue_ref(*(HValue *)vector_last(&m->opstack));
}

static inline void opstack_push(Machine *m, HValue value) {
    *(HValue *)vector_push(&m->opstack) = value;
}

static inline string read_string(Machine *m) {
    uint32_t offset = read_u32be(&m->ip);
    uint32_t len = read_u32be(&m->ip);

    return (string){
        .data = m->prog->string_bytes + offset,
        .len = len,
    };
}

static HValue make_closure(Machine *m, uint32_t fnindex) {
    func_meta fn;
    program_func_meta(m->prog, fnindex, &fn);

    // TODO
}

static void call_closure(Machine *m, uint8_t args) {
    HValue closure;
    debug_assert(vector_pop(&m->opstack, &closure));

    // TODO

    hvalue_drop(closure);
}

bool machine_step(Machine *m) {
    HValue value;

    const uint8_t *op = m->ip++;

    switch (*op) {
    case OP_JUMP: m->ip = op + read_i16be(&m->ip); return true;
    case OP_JUMPIF:
        debug_assert(vector_pop(&m->opstack, &value));
        if (value.type == V_TRUE)
            m->ip = op + read_i16be(&m->ip);
        else
            debug_assert(value.type == V_FALSE);
        hvalue_drop(value);
        return true;
    case OP_JUMPIFN:
        debug_assert(vector_pop(&m->opstack, &value));
        if (value.type == V_FALSE)
            m->ip = op + read_i16be(&m->ip);
        else
            debug_assert(value.type == V_TRUE);
        hvalue_drop(value);
        return true;
    case OP_CALL: push_frame(m, read_u32be(&m->ip)); return true;
    case OP_RETURN: pop_frame(m); return true;
    case OP_LOAD: load_local(m, *m->ip++); return true;
    case OP_STORE: store_local(m, *m->ip++); return true;
    case OP_DUP: opstack_dup(m); return true;
    case OP_POP:
        debug_assert(vector_pop(&m->opstack, &value));
        hvalue_drop(value);
        return true;
    case OP_PUSHINT: opstack_push(m, hvalue_make_int(read_i64be(&m->ip))); return true;
    case OP_PUSHSTR: opstack_push(m, hvalue_make_string(read_string(m))); return true;
    case OP_PUSHTRUE: opstack_push(m, hvalue_make(V_TRUE)); return true;
    case OP_PUSHFALSE: opstack_push(m, hvalue_make(V_FALSE)); return true;
    case OP_MAKECLS: opstack_push(m, make_closure(m, read_u32be(&m->ip))); return true;
    case OP_CALLCLS: call_closure(m, *m->ip++); return true;
    case OP_HALT: return false;
    default:
        panic(
            "unsupported opcode: %02" PRIX8 " @ %08" PRIX32, *op, (uint32_t)(op - m->prog->bytecode)
        );
    }
}
