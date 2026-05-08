#include "prelude.h"

#include "each.h"
#include "function.h"
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

void module_make_prelude(Module *m) {
    module_define(m, STRING("id"), hnative_make_id());
    module_define(m, STRING("const"), hnative_make_const());

    module_define(m, STRING("print"), hnative_make_print());
    module_define(m, STRING("get_time"), hnative_make_get_time());
    module_define(m, STRING("each"), hnative_make_each());

    module_define(m, STRING("+"), hnative_make_add());
    module_define(m, STRING("-"), hnative_make_sub());
    module_define(m, STRING("*"), hnative_make_mul());
    module_define(m, STRING("/"), hnative_make_div());

    module_define(m, STRING("++"), hnative_make_string_concat());
}
