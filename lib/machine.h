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

void machine_init(Machine *, const program *);
void machine_free(Machine *);
bool machine_step(Machine *);

#endif // HAMMER_MACHINE_H_
