#include "prelude.h"

#include "each.h"
#include "function.h"
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

void module_make_prelude(Module *m, Parser *p) {
    module_define(m, STRING("id"), hnative_make_id);
    module_define(m, STRING("const"), hnative_make_const);

    module_define(m, STRING("print"), hnative_make_print);
    module_define(m, STRING("get_time"), hnative_make_get_time);
    module_define(m, STRING("each"), hnative_make_each);

    parser_define_operator(p, STRING("+"), 500, ASSOC_LEFT);
    module_define(m, STRING("+"), hnative_make_add);
    parser_define_operator(p, STRING("-"), 500, ASSOC_LEFT);
    module_define(m, STRING("-"), hnative_make_sub);
    parser_define_operator(p, STRING("*"), 600, ASSOC_LEFT);
    module_define(m, STRING("*"), hnative_make_mul);
    parser_define_operator(p, STRING("/"), 600, ASSOC_LEFT);
    module_define(m, STRING("/"), hnative_make_div);

    parser_define_operator(p, STRING("++"), 500, ASSOC_LEFT);
    module_define(m, STRING("++"), hnative_make_string_concat);
    module_define(m, STRING("substr"), hnative_make_string_substr);
}
