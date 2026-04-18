#ifndef COMPILER_H_
#define COMPILER_H_

#include "ast.h"
#include "stack.h"

typedef struct {
    Buffer string_buffer;
    Stack  procs;
    Stack  frames;
} Compiler;

void compiler_init(Compiler *);
void compiler_free(Compiler *);
void compiler_visit(Compiler *, node *);
void compiler_write_program(Compiler *, Buffer *);

#endif // COMPILER_H_
