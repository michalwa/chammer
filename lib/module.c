#include "module.h"

#include "string_map.h"
#include "value.h"

void module_init(Module *m) {
    string_map_init(&m->members, HValue);
}

void module_free(Module *m) {
    for (string_map_iter i = string_map_iter_begin(&m->members); string_map_iter_next(&i);)
        hvalue_drop(*(HValue *)i.entry->value);

    string_map_free(&m->members);
}

void module_define(Module *m, string name, HValue value) {
    string_map_put(&m->members, name, &value, NULL);
}

inline void module_define_native(Module *m, HValue value) {
    module_define(m, string_from_cstr(hvalue_native_name(&value)), value);
}

bool module_get(Module *m, string name, const HValue **value) {
    const string_map_entry *entry = string_map_get_entry(&m->members, name);
    if (!entry) return false;
    *value = (const HValue *)entry->value;
    return true;
}
