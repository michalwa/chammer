#ifndef HAMMER_BUILTIN_STRING_H_
#define HAMMER_BUILTIN_STRING_H_

#include "../value.h"

extern hnative_meta HNATIVE_META_REPR;
extern hnative_meta HNATIVE_META_STRING_CONCAT;
extern hnative_meta HNATIVE_META_SUBSTR;

HValue hnative_make_repr(void);
HValue hnative_make_string_concat(void);
HValue hnative_make_substr(void);

#endif // HAMMER_BUILTIN_STRING_H_
