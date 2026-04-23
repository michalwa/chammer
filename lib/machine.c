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

bool machine_step(Machine *m) {
    vm_value *v;

    switch (*m->ip) {
    case OP_JUMP:
        m->ip++;
        m->ip = m->program + read_u32be(m);
        break;
    case OP_PUSHINT:
        m->ip++;
        v = (vm_value *)stack_push(&m->opstack);
        v->type = V_INT;
        v->value.int_value = read_u64be(m);
        break;
    case OP_HALT:
        return false;
    default:
        panic("unsupported opcode: %02X", *m->ip);
    }

    return true;
}
