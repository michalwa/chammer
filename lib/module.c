#include "module.h"

#include "string_map.h"
#include "value.h"

struct ModuleMember {
    hvalue_factory factory;
    HValue         value;
};

void module_init(Module *m) {
    string_map_init(&m->members, ModuleMember);
}

void module_free(Module *m) {
    for (string_map_iter i = string_map_iter_begin(&m->members); string_map_iter_next(&i);) {
        ModuleMember *member = (ModuleMember *)i.entry->value;
        if (member->value.type) hvalue_drop(member->value);
    }

    string_map_free(&m->members);
}

void module_define(Module *m, string name, hvalue_factory factory) {
    string_map_put(&m->members, name, &(ModuleMember){ factory, { 0 } }, NULL);
}

const HValue *module_get(Module *m, string name) {
    const string_map_entry *entry = string_map_get_entry(&m->members, name);
    if (!entry) return NULL;

    ModuleMember *member = (ModuleMember *)entry->value;
    if (!member->value.type) member->value = member->factory();

    return &member->value;
}
