#include "prelude.h"

#include "each.h"
#include "function.h"
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

void module_make_prelude(Module *m) {
    module_define_native(m, hnative_make_id());
    module_define_native(m, hnative_make_const());
    module_define_native(m, hnative_make_print());
    module_define_native(m, hnative_make_get_time());
    module_define_native(m, hnative_make_each());
    module_define_native(m, hnative_make_add());
    module_define_native(m, hnative_make_sub());
    module_define_native(m, hnative_make_mul());
    module_define_native(m, hnative_make_div());
    module_define_native(m, hnative_make_string_concat());
}
