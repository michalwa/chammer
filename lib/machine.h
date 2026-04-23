#ifndef MACHINE_H_
#define MACHINE_H_

#include <stdbool.h>

#include "buffer.h"
#include "stack.h"

typedef struct {
    Buffer   fnstack;
    Stack    opstack;
    uint8_t *program;
    size_t   program_size;
    uint8_t *ip;
} Machine;

#define EACH_VM_VALUE_TYPE(_) \
    _(V_INT)                  \
    _(V_FLOAT)                \
    _(V_STRING)               \
    _(V_BOOL)

#define ENUM_MEMBER(name) name,
typedef enum { EACH_VM_VALUE_TYPE(ENUM_MEMBER) } vm_value_type;
#undef ENUM_MEMBER

typedef struct {
    vm_value_type type;
    union {
        // TODO: make this `int64_t` obviously
        uint64_t int_value;
        double   float_value;
        bool     bool_value;
    } value;
} vm_value;

void machine_init(Machine *, uint8_t *program, size_t program_size);
void machine_free(Machine *);
bool machine_step(Machine *);

#endif // MACHINE_H_
