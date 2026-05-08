#ifndef HAMMER_BUILTIN_MATH_H_
#define HAMMER_BUILTIN_MATH_H_

#include "../value.h"

extern hnative_meta HNATIVE_META_ADD;
extern hnative_meta HNATIVE_META_SUB;
extern hnative_meta HNATIVE_META_MUL;
extern hnative_meta HNATIVE_META_DIV;

HValue hnative_make_add(void);
HValue hnative_make_sub(void);
HValue hnative_make_mul(void);
HValue hnative_make_div(void);

#endif // HAMMER_BUILTIN_MATH_H_
