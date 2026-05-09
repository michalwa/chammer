#ifndef HAMMER_MODULE_H_
#define HAMMER_MODULE_H_

#include "string_map.h"

typedef struct HValue HValue; // forward declaration, `HValue` is defined in `value.h`

typedef HValue (*hvalue_factory)(void);

typedef struct ModuleMember ModuleMember;

// NOTE: Struct declared with name to resolve circular dependencies
typedef struct Module {
    StringMap members; // value type: ModuleMember
} Module;

void          module_init(Module *);
void          module_free(Module *);
void          module_define(Module *, string name, hvalue_factory value);
const HValue *module_get(Module *, string name);

#endif // HAMMER_MODULE_H_
