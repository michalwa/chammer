#ifndef HAMMER_COMPILER_H_
#define HAMMER_COMPILER_H_

#include "arena.h"
#include "ast.h"
#include "buffer.h"
#include "string_pool.h"
#include "vector.h"

typedef struct {
    Buffer     string_buffer;
    StringPool strings;
    StringPool idents;
    Arena      blocks;
    Vector     jumps;
    Vector     funcs;
} Compiler;

void compiler_init(Compiler *);
void compiler_free(Compiler *);
void compiler_visit_program(Compiler *, node *);
void compiler_write_program(Compiler *, Buffer *);

int64_t compile_int(string);
/*
 * Expects the full contents of a string token, including quotes
 */
void    compile_string(string, Buffer *);

#endif // HAMMER_COMPILER_H_
