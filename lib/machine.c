#include "machine.h"

/*
 * Function call frame header (stack pointer at bottom)
 */
typedef struct {
    // value locals[]; (locals in reverse order)
    uint32_t return_sp;
    uint32_t return_ip;
    uint32_t fnindex;
} frame;

void machine_init(Machine *m, const program *prog) {
    m->prog = prog;
    buffer_init(&m->fnstack);
    vector_init(&m->opstack);
    m->ip = 0;
}

void machine_free(Machine *m) {
    buffer_free(&m->fnstack);
    vector_init(&m->opstack);
}

bool machine_step(Machine *m) {
    switch (*m->ip) {
    case OP_HALT:
        return false;
    default:
        panic("unsupported opcode: %02" PRIX8, *m->ip);
    }

    return true;
}
