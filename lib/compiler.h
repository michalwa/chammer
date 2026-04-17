#ifndef COMPILER_H_
#define COMPILER_H_

#include "stack.h"
#include "ast.h"

typedef struct {
    Buffer string_buffer;
    Buffer bytecode;
} Compiler;

void compiler_init(Compiler *);
void compiler_free(Compiler *);
void compiler_visit(Compiler *, node *);
void compiler_write_program(Compiler *, Buffer *);

#endif // COMPILER_H_
