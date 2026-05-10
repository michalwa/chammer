#ifndef HAMMER_MACHINE_H_
#define HAMMER_MACHINE_H_

#include "buffer.h"
#include "bytecode.h"
#include "module.h"
#include "utils.h"
#include "vector.h"

#define machine_call(ctx, callee, ...)                                       \
    machine_call_(ctx, callee, ARGC(__VA_ARGS__), (HValue[]){ __VA_ARGS__ })

typedef struct HValue HValue; // forward declaration, `HValue` is defined in `value.h`

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
    Vector         modules;
    /*
     * Multipurpose buffer that is free to use by native value implementations
     */
    Buffer         shared_buffer;
} Machine;

void machine_init(Machine *, const program *);
void machine_free(Machine *);
void machine_add_module(Machine *, Module);
bool machine_step(Machine *, HValue *error);

string machine_func_name(const Machine *, uint32_t);
/*
 * Initiates a call of the given callable value and advances the machine until
 * the call returns. Pops the result off the stack and returns it.
 */
HValue machine_call_(Machine *, HValue callee, size_t argc, HValue *args);

#endif // HAMMER_MACHINE_H_
