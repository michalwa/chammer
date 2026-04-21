#ifndef COMPILER_H_
#define COMPILER_H_

#include "ast.h"
#include "stack.h"
#include "vector.h"

typedef struct {
    Buffer string_buffer;
    Vector blocks;
    Vector frames;
    Vector jumps;
    Vector traces;
} Compiler;

void compiler_init(Compiler *);
void compiler_free(Compiler *);
void compiler_visit_program(Compiler *, node *);
void compiler_write_program(Compiler *, Buffer *);

void compile_string(string, Buffer *, size_t *offset, size_t *len);

#endif // COMPILER_H_
