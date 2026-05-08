#ifndef HAMMER_MODULE_H_
#define HAMMER_MODULE_H_

#include "string_map.h"

typedef struct HValue HValue; // forward declaration, `HValue` is defined in `value.h`

// NOTE: Struct declared with name to resolve circular dependencies
typedef struct Module {
    StringMap members; // value type: HValue
} Module;

void module_init(Module *);
void module_free(Module *);
void module_define(Module *, string name, HValue value);
bool module_get(Module *, string name, const HValue **value);

#endif // HAMMER_MODULE_H_
