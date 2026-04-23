#include "machine.h"

#include "bytecode.h"
#include "utils.h"

void machine_init(Machine *m, uint8_t *program, size_t program_size) {
    buffer_init(&m->fnstack);
    stack_init(&m->opstack, vm_value);
    m->ip = m->program = program;
    m->program_size = program_size;
}

void machine_free(Machine *m) {
    buffer_free(&m->fnstack);
    stack_free(&m->opstack);
}

static uint32_t read_u32be(Machine *m) {
    debug_assert(m->ip - m->program + sizeof(u32be) <= m->program_size);
    uint32_t value = u32be_value(*(u32be *)m->ip);
    m->ip += sizeof(u32be);
    return value;
}

static uint64_t read_u64be(Machine *m) {
    debug_assert(m->ip - m->program + sizeof(u64be) <= m->program_size);
    uint64_t value = u64be_value(*(u64be *)m->ip);
    m->ip += sizeof(u64be);
    return value;
}

static inline uint8_t *fnstack_top(Machine *m) {
    return (uint8_t *)(m->fnstack.data + m->fnstack.len);
}

static inline size_t local_offset(uint8_t i) {
    return sizeof(uint32_t) + (i + 1) * sizeof(vm_value);
}

static void push_frame(Machine *m, uint8_t locals) {
    buffer_alloc(&m->fnstack, (size_t)locals * sizeof(vm_value) + sizeof(uint32_t));
    uint32_t return_addr = (uint32_t)(m->ip - m->program);
    *(uint32_t *)(fnstack_top(m) - sizeof(uint32_t)) = return_addr;
}

static inline void push_operand(Machine *m, vm_value v) {
    *(vm_value *)stack_push(&m->opstack) = v;
}

static vm_value pop_operand(Machine *m) {
    vm_value v = *(vm_value *)stack_top(&m->opstack);
    stack_pop(&m->opstack);
    return v;
}

static void load_local(Machine *m, uint8_t i) {
    vm_value *local = (vm_value *)(fnstack_top(m) - local_offset(i));
    push_operand(m, *local);
}

static void store_local(Machine *m, uint8_t i) {
    vm_value *local = (vm_value *)(fnstack_top(m) - local_offset(i));
    *local = pop_operand(m);
}

bool machine_step(Machine *m) {
    vm_value v1, v2;

    switch (*m->ip) {
    case OP_JUMP:
        m->ip++;
        m->ip = m->program + read_u32be(m);
        break;
    case OP_CALL:
        m->ip++;
        push_frame(m, *m->ip++);
        m->ip = m->program + read_u32be(m);
        break;
    case OP_LOAD:
        m->ip++;
        load_local(m, *m->ip++);
        break;
    case OP_STORE:
        m->ip++;
        store_local(m, *m->ip++);
        break;
    case OP_PUSHINT:
        m->ip++;
        v1.type = V_INT;
        v1.value.int_value = read_u64be(m);
        push_operand(m, v1);
        break;
    case OP_ADD:
        m->ip++;
        v1 = pop_operand(m);
        v2 = pop_operand(m);
        v1.type = V_INT;
        v1.value.int_value += v2.value.int_value;
        push_operand(m, v1);
        break;
    case OP_HALT: return false;
    default: panic("unsupported opcode: %02" PRIX8, *m->ip);
    }

    return true;
}
