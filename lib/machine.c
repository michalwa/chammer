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
    *(HValue *)vector_push(&m->opstack) = hvalue_ref(get_local(m, i));
}

static inline void store_local(Machine *m, uint8_t i) {
    debug_assert(vector_pop(&m->opstack, get_local(m, i)));
}

static inline void opstack_dup(Machine *m) {
    *(HValue *)vector_push(&m->opstack) = hvalue_ref((HValue *)vector_last(&m->opstack));
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

    HValue hv_closure = hvalue_make_closure(fnindex, fn.captures + fn.args);

    for (uint8_t i = 0; i < fn.captures; i++) {
        HValue arg;
        debug_assert(vector_pop(&m->opstack, &arg));
        hvalue_closure_put_arg_mut(&hv_closure, arg);
    }

    return hv_closure;
}

static void call_closure(Machine *m, uint8_t args) {
    HValue hv_closure;
    debug_assert(vector_pop(&m->opstack, &hv_closure));
    hv_closure = hvalue_uniq(hv_closure);

    const HClosure *closure;
    debug_assert(hvalue_get_closure(&hv_closure, &closure));

    func_meta fn;
    program_func_meta(m->prog, closure->fnindex, &fn);

    debug_assert(closure->args_len + args <= fn.captures + fn.args);

    if (closure->args_len + args < fn.captures + fn.args) {
        for (uint8_t i = 0; i < args; i++) {
            HValue arg;
            debug_assert(vector_pop(&m->opstack, &arg));
            hvalue_closure_put_arg_mut(&hv_closure, arg);
        }

        opstack_push(m, hv_closure);
    } else {
        HValue arg;
        while (hvalue_closure_take_arg_mut(&hv_closure, &arg)) opstack_push(m, arg);

        push_frame(m, closure->fnindex);
        hvalue_drop(hv_closure);
    }
}

static void check_tuple(Machine *m, uint16_t len) {
    HValue hv_tuple;
    debug_assert(vector_pop(&m->opstack, &hv_tuple));

    if (hv_tuple.type == V_TUPLE) {
        const HTuple *tuple;
        debug_assert(hvalue_get_tuple(&hv_tuple, &tuple));

        opstack_push(m, hvalue_make_bool(tuple->len == len));
    } else {
        opstack_push(m, hvalue_make(V_FALSE));
    }

    hvalue_drop(hv_tuple);
}

static void tuple_get(Machine *m, uint16_t i) {
    HValue hv_tuple;
    debug_assert(vector_pop(&m->opstack, &hv_tuple));

    const HTuple *tuple;
    debug_assert(hvalue_get_tuple(&hv_tuple, &tuple));

    opstack_push(m, hvalue_ref(&tuple->data[i]));

    hvalue_drop(hv_tuple);
}

static void make_tuple(Machine *m, uint16_t len) {
    HTupleBuilder tb = htuple_begin(len);

    for (uint16_t i = 0; i < len; i++) {
        HValue item;
        debug_assert(vector_pop(&m->opstack, &item));
        htuple_put(&tb, item);
    }

    opstack_push(m, htuple_end(tb));
}

static void make_list(Machine *m, uint16_t len) {
    opstack_push(m, hvalue_make(V_NIL));

    for (uint16_t i = 0; i < len; i++) {
        HValue tail, head;
        debug_assert(vector_pop(&m->opstack, &tail));
        debug_assert(vector_pop(&m->opstack, &head));

        opstack_push(m, hvalue_make_cons(head, tail));
    }
}

static void uncons(Machine *m) {
    HValue hv_cons;
    debug_assert(vector_pop(&m->opstack, &hv_cons));

    const HCons *cons;
    debug_assert(hvalue_get_cons(&hv_cons, &cons));

    opstack_push(m, hvalue_ref(&cons->tail));
    opstack_push(m, hvalue_ref(&cons->head));

    hvalue_drop(hv_cons);
}

static void check_type(Machine *m, hvalue_type type) {
    const HValue *value = vector_last(&m->opstack);
    opstack_push(m, hvalue_make_bool(value->type == type));
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
    case OP_ISTUPLE: check_tuple(m, read_u16be(&m->ip)); return true;
    case OP_TUPLEGET: tuple_get(m, read_u16be(&m->ip)); return true;
    case OP_MAKETUPLE: make_tuple(m, read_u16be(&m->ip)); return true;
    case OP_MAKELIST: make_list(m, read_u16be(&m->ip)); return true;
    case OP_ISNIL: check_type(m, V_NIL); return true;
    case OP_ISCONS: check_type(m, V_CONS); return true;
    case OP_UNCONS: uncons(m); return true;
    case OP_HALT: return false;
    default:
        panic(
            "unsupported opcode: %02" PRIX8 " @ %08" PRIX32, *op, (uint32_t)(op - m->prog->bytecode)
        );
    }
}
