#ifndef HAMMER_MACHINE_H_
#define HAMMER_MACHINE_H_

#include "buffer.h"
#include "bytecode.h"
#include "vector.h"

typedef struct {
    const program *prog;
    /*
     * Function/call stack
     */
    Buffer         fnstack;
    /*
     * Operand stack (item type: HValue)
     */
    Vector         opstack;
    const uint8_t *ip;
} Machine;

typedef struct {
    const program *prog;
} machine_ctx;

void machine_init(Machine *, const program *);
void machine_free(Machine *);
bool machine_step(Machine *);

void   machine_ctx_init(machine_ctx *, const Machine *);
string machine_ctx_func_name(const machine_ctx *, uint32_t);

#endif // HAMMER_MACHINE_H_
