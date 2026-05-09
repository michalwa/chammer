#include "machine.h"

#include "bytecode.h"
#include "bytes.h"
#include "module.h"
#include "utils.h"
#include "value.h"
#include "vector.h"

#define vm_debug_assert(m, expr)                                                               \
    debug_assert_msg(                                                                          \
        expr, "ip: %08" PRIX32 ", sp: %08X" PRIX32, (uint32_t)((m)->ip - (m)->prog->bytecode), \
        (uint32_t)(m)->opstack.len                                                             \
    )

/*
 * Function call frame header (stack pointer at bottom)
 */
typedef struct {
    // HValue locals[]; (locals in reverse order)
    uint32_t return_sp;
    uint32_t return_ip;
    uint32_t fnindex;
    char     padding_[4]; // HValue array requires 8-byte alignment
} frame;

struct machine_ctx {
    const program *prog;
};

static inline void pop_frame(Machine *);

void machine_init(Machine *m, const program *prog) {
    m->prog = prog;
    buffer_init(&m->fnstack);
    vector_init(&m->opstack, HValue);
    vector_init(&m->modules, Module);
    m->ip = prog->bytecode;
}

void machine_free(Machine *m) {
    while (m->fnstack.len > 0) pop_frame(m);

    for (EACH_IN_VECTOR(m->opstack, HValue, value)) hvalue_drop(*value);
    for (EACH_IN_VECTOR(m->modules, Module, module)) module_free(module);

    buffer_free(&m->fnstack);
    vector_free(&m->opstack);
    vector_free(&m->modules);
}

void machine_add_module(Machine *m, Module module) {
    *(Module *)vector_push(&m->modules) = module;
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

    size_t num_locals = (m->fnstack.len - f->return_sp - sizeof(frame)) / sizeof(HValue);
    for (size_t i = 0; i < num_locals; i++) hvalue_drop(*get_local(m, i));

    m->ip = m->prog->bytecode + f->return_ip;
    buffer_truncate(&m->fnstack, f->return_sp);
}

static inline void op_load(Machine *m, uint8_t i) {
    *(HValue *)vector_push(&m->opstack) = hvalue_ref(get_local(m, i));
}

static inline void op_store(Machine *m, uint8_t i) {
    vm_debug_assert(m, vector_pop(&m->opstack, get_local(m, i)));
}

static inline void op_dup(Machine *m) {
    const HValue *last = (HValue *)vector_last(&m->opstack);
    *(HValue *)vector_push(&m->opstack) = hvalue_ref(last);
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
        vm_debug_assert(m, vector_pop(&m->opstack, &arg));
        hvalue_closure_put_arg_mut(&hv_closure, arg);
    }

    return hv_closure;
}

static void call_closure(Machine *m, HValue closure, uint8_t args) {
    closure = hvalue_uniq(closure);

    if (args < hvalue_closure_args_left(&closure)) {
        for (uint8_t i = 0; i < args; i++) {
            HValue arg;
            vm_debug_assert(m, vector_pop(&m->opstack, &arg));
            hvalue_closure_put_arg_mut(&closure, arg);
        }

        opstack_push(m, closure);
    } else {
        debug_assert(args == hvalue_closure_args_left(&closure));

        // If after pushing `args` the closure would be ready to call, don't bother
        // storing args in it, but rather pop the rest onto the stack and call it
        HValue arg;
        while (hvalue_closure_take_arg_mut(&closure, &arg)) opstack_push(m, arg);

        push_frame(m, hvalue_closure_fnindex(&closure));
        hvalue_drop(closure);
    }
}

static void call_native(Machine *m, HValue native, uint8_t args) {
    native = hvalue_uniq(native);

    for (uint8_t i = 0; i < args; i++) {
        HValue arg;
        vm_debug_assert(m, vector_pop(&m->opstack, &arg));
        hvalue_native_put_arg_mut(&native, arg);
    }

    if (hvalue_native_args_left(&native)) {
        opstack_push(m, native);
    } else {
        opstack_push(m, hvalue_native_call(&native, m));
        hvalue_drop(native);
    }
}

static void op_callval(Machine *m, uint8_t args) {
    HValue value;
    vm_debug_assert(m, vector_pop(&m->opstack, &value));

    switch (value.type) {
    case V_CLOSURE: call_closure(m, value, args); break;
    case V_NATIVE: call_native(m, value, args); break;
    default: panic("value of type %s is not callable", hvalue_type_name(value.type));
    }
}

static void op_chktuple(Machine *m, uint16_t len) {
    HValue *peek = (HValue *)vector_last(&m->opstack);

    if (peek->type != V_TUPLE || hvalue_tuple_len(peek) != len) {
        HValue tuple;
        vector_pop(&m->opstack, &tuple);
        hvalue_drop(tuple);

        opstack_push(m, hvalue_make(V_FALSE));
    } else {
        opstack_push(m, hvalue_make(V_TRUE));
    }
}

static void op_check_type(Machine *m, hvalue_type type) {
    HValue *peek = (HValue *)vector_last(&m->opstack);

    if (peek->type != type) {
        HValue value;
        vector_pop(&m->opstack, &value);
        hvalue_drop(value);

        opstack_push(m, hvalue_make(V_FALSE));
    } else {
        opstack_push(m, hvalue_make(V_TRUE));
    }
}

static void op_tupleget(Machine *m, uint16_t i) {
    opstack_push(m, hvalue_tuple_get((HValue *)vector_last(&m->opstack), i));
}

static void op_maketuple(Machine *m, uint16_t len) {
    HTupleBuilder tb = htuple_begin(len);

    for (uint16_t i = 0; i < len; i++) {
        HValue item;
        vm_debug_assert(m, vector_pop(&m->opstack, &item));
        htuple_put(&tb, item);
    }

    opstack_push(m, htuple_end(tb));
}

static void op_makelist(Machine *m, uint16_t len) {
    opstack_push(m, hvalue_make(V_NIL));

    for (uint16_t i = 0; i < len; i++) {
        HValue tail, head;
        vm_debug_assert(m, vector_pop(&m->opstack, &tail));
        vm_debug_assert(m, vector_pop(&m->opstack, &head));

        opstack_push(m, hvalue_make_cons(head, tail));
    }
}

static void op_uncons(Machine *m) {
    HValue cons, head, tail;
    vm_debug_assert(m, vector_pop(&m->opstack, &cons));

    hvalue_uncons(cons, &head, &tail);

    opstack_push(m, tail);
    opstack_push(m, head);
}

static void load_extern(Machine *m, string name) {
    const HValue *value;

    for (EACH_IN_VECTOR(m->modules, Module, module)) {
        if (value = module_get(module, name)) {
            opstack_push(m, hvalue_ref(value));
            return;
        }
    }

    panic("unresolved symbol: " F_STRING, FA_STRING(name));
}

static void op_concat(Machine *m) {
    HValue a, b;
    vm_debug_assert(m, vector_pop(&m->opstack, &b));
    vm_debug_assert(m, vector_pop(&m->opstack, &a));

    opstack_push(m, hvalue_list_concat(a, b));
}

static void op_bind(Machine *m) {
    HValue monad, then;
    vm_debug_assert(m, vector_pop(&m->opstack, &monad));
    vm_debug_assert(m, vector_pop(&m->opstack, &then));

    opstack_push(m, hvalue_bind(monad, then, m));
}

static void op_yield(Machine *m) {
    HValue effect;
    vm_debug_assert(m, vector_pop(&m->opstack, &effect));

    hvalue_yield(&effect, m, NULL);
}

static void op_eq(Machine *m) {
    HValue a, b;
    vm_debug_assert(m, vector_pop(&m->opstack, &a));
    vm_debug_assert(m, vector_pop(&m->opstack, &b));

    opstack_push(m, hvalue_make_bool(hvalue_eq(&a, &b)));

    hvalue_drop(a);
    hvalue_drop(b);
}

bool machine_step(Machine *m) {
    HValue  value;
    int16_t jump;

    const uint8_t *op = m->ip++;

    switch (*op) {
    case OP_JUMP: m->ip = op + read_i16be(&m->ip); return true;
    case OP_JUMPIF:
        jump = read_i16be(&m->ip);
        vm_debug_assert(m, vector_pop(&m->opstack, &value));
        if (value.type == V_TRUE)
            m->ip = op + jump;
        else
            vm_debug_assert(m, value.type == V_FALSE);
        hvalue_drop(value);
        return true;
    case OP_JUMPIFN:
        jump = read_i16be(&m->ip);
        vm_debug_assert(m, vector_pop(&m->opstack, &value));
        if (value.type == V_FALSE)
            m->ip = op + jump;
        else
            vm_debug_assert(m, value.type == V_TRUE);
        hvalue_drop(value);
        return true;
    case OP_CALL: push_frame(m, read_u32be(&m->ip)); return true;
    case OP_RETURN: pop_frame(m); return true;
    case OP_LOAD: op_load(m, *m->ip++); return true;
    case OP_STORE: op_store(m, *m->ip++); return true;
    case OP_DUP: op_dup(m); return true;
    case OP_POP:
        vm_debug_assert(m, vector_pop(&m->opstack, &value));
        hvalue_drop(value);
        return true;
    case OP_SWAP:
        vm_debug_assert(m, m->opstack.len >= 2);
        value = *(HValue *)vector_last(&m->opstack);
        *(HValue *)vector_last(&m->opstack) =
            *(HValue *)vector_get(&m->opstack, m->opstack.len - 2);
        *(HValue *)vector_get(&m->opstack, m->opstack.len - 2) = value;
        return true;
    case OP_PUSHINT: opstack_push(m, hvalue_make_int(read_i64be(&m->ip))); return true;
    case OP_PUSHSTR: opstack_push(m, hvalue_make_string(read_string(m))); return true;
    case OP_PUSHTRUE: opstack_push(m, hvalue_make(V_TRUE)); return true;
    case OP_PUSHFALSE: opstack_push(m, hvalue_make(V_FALSE)); return true;
    case OP_MAKECLS: opstack_push(m, make_closure(m, read_u32be(&m->ip))); return true;
    case OP_CALLVAL: op_callval(m, *m->ip++); return true;
    case OP_BIND: op_bind(m); return true;
    case OP_CHKTUPLE: op_chktuple(m, read_u16be(&m->ip)); return true;
    case OP_TUPLEGET: op_tupleget(m, read_u16be(&m->ip)); return true;
    case OP_MAKETUPLE: op_maketuple(m, read_u16be(&m->ip)); return true;
    case OP_MAKELIST: op_makelist(m, read_u16be(&m->ip)); return true;
    case OP_CHKNIL: op_check_type(m, V_NIL); return true;
    case OP_CHKCONS: op_check_type(m, V_CONS); return true;
    case OP_UNCONS: op_uncons(m); return true;
    case OP_CONCAT: op_concat(m); return true;
    case OP_EQ: op_eq(m); return true;
    case OP_LOADEXT:
        vm_debug_assert(m, vector_pop(&m->opstack, &value));
        load_extern(m, hvalue_string_get(&value));
        hvalue_drop(value);
        return true;
    case OP_YIELD: op_yield(m); return false;
    case OP_HALT: return false;
    default:
        panic(
            "unsupported opcode: %02" PRIX8 " @ %08" PRIX32, *op, (uint32_t)(op - m->prog->bytecode)
        );
    }
}

string machine_func_name(const Machine *m, uint32_t fnindex) {
    func_meta fn;
    program_func_meta(m->prog, fnindex, &fn);
    return program_func_name(m->prog, &fn);
}

HValue machine_call_(Machine *m, HValue callee, size_t argc, HValue *args) {
    for (size_t i = 0; i < argc; i++) opstack_push(m, args[argc - 1 - i]);

    size_t initial_sp = m->fnstack.len;

    opstack_push(m, callee);
    op_callval(m, argc);

    while (m->fnstack.len > initial_sp && machine_step(m));

    HValue result;
    vm_debug_assert(m, vector_pop(&m->opstack, &result));
    return result;
}
