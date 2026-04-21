#ifndef COMPILER_H_
#define COMPILER_H_

#include "ast.h"
#include "buffer.h"
#include "string_pool.h"
#include "vector.h"

typedef struct {
    Buffer     string_buffer;
    StringPool strings;
    Vector     blocks;
    Vector     frames;
    Vector     jumps;
    Vector     traces;
    StringPool idents;
} Compiler;

void compiler_init(Compiler *);
void compiler_free(Compiler *);
void compiler_visit_program(Compiler *, node *);
void compiler_write_program(Compiler *, Buffer *);

void compile_string(string, Buffer *);

#endif // COMPILER_H_
