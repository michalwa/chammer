#ifndef HAMMER_BUILTIN_FUNCTION_H_
#define HAMMER_BUILTIN_FUNCTION_H_

#include "../value.h"

extern hnative_meta HNATIVE_META_ID;
extern hnative_meta HNATIVE_META_CONST;

HValue hnative_make_id(void);
HValue hnative_make_const(void);

#endif // HAMMER_BUILTIN_FUNCTION_H_
