#ifndef COMPILER_H_
#define COMPILER_H_

#include "ast.h"
#include "stack.h"

typedef struct {
    Buffer string_buffer;
    Stack  procs;
    Stack  frames;
    Stack  jumps;
} Compiler;

void compiler_init(Compiler *);
void compiler_free(Compiler *);
void compiler_visit(Compiler *, node *);
void compiler_write_program(Compiler *, Buffer *);

void compile_string(string, Buffer *, size_t *offset, size_t *len);

#endif // COMPILER_H_
