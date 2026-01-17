# Style

The following are code style guidelines that have not yet been translated to compiler warnings or formatter configuration.

## Naming

## Abbreviations

Abbreviations are permitted so long as they're unambiguous. Short names are preferred to overly verbose ones, so long as they remain practical.

For example, the name `N_DOBLK` for a constant is ok (as opposed to `NODE_TYPE_DO_BLOCK`), because the constituent words are not relevant to its meaning or usage and it unambiguously refers to a concept (in this case an AST node type). Also note that `DOBLK` is formatted as a single word, because again, distinguishing between the constituent words is not relevant.

Single-character abbreviations are also permitted in functions associated with a type, operating on a single instance of that type:

```c
typedef struct { /* ... */ } Object;

void object_do_something(Object *o) { /* ... */ }
```

## Type name case

Use `snake_case` for primitive/copyable types and `PascalCase` for non-copyable types, i.e. types which allocate resources.

```c
typedef char three_bytes[3];
typedef struct { int i, j; } vec2i;
typedef struct { char *data; size_t len; } String;
```

## Macro name case

Macros should generally use `UPPER_CASE`. Permissible exceptions are function-like macros which are wrappers around functions, e.g. for passing variadic argument count or type information.

- Macros containing control flow statements **must** use `UPPER_CASE`.
- Macros referring to local symbols which are not macro parameters **must** use `UPPER_CASE`.

```c
// good
#define CONSTANT 42
#define ALLOCATE(t) allocate_(sizeof(t))
#define FAIL do { return 1 } while (0)

// ok
#define allocate(t) allocate_(sizeof(t))

// bad
#define constant 42
#define fail do { return 1 } while (0)
```

### Standard names

| Format   | Item        | Example                      |
| -------- | ----------- | ---------------------------- |
| `*_init` | constructor | `void object_init(Object *)` |
| `*_free` | destructor  | `void object_free(Object *)` |

## Declarations

### `typedef` names

For a `struct` or `enum` type defined inline in a `typedef` declaration, do not specify the name for the `struct` or `enum`.

Permissible exceptions are:

- self-referential `struct`-s,
- when the type is left opaque in the header file and only defined in the implementation file.

```c
// good
typedef struct { /* ... */ } Foo;

// ok
typedef struct node node;
struct node { node *next; };

// bad
typedef struct Foo { /* ... */ } Foo;
```

```c
// good

// foo.h
typedef struct Foo Foo;
// foo.c
struct Foo { /* ... */ };
```

## Macros

### Expression safety

- Always wrap macro arguments in parentheses when used as operands.
- Always wrap function-like macros in parentheses.

```c
// good
#define SUM(a, b) ((a) + (b))
#define MUL(a, b) ((a) * (b))
SUM(x, y) * z // ((x) + (y)) * z
MUL(x, y + z) // ((x) * (y + z))

// bad
#define SUM(a, b) a + b
#define MUL(a, b) a * b
SUM(x, y) * z // x + y * z
MUL(x, y + z) // x * y + z
```

### Statement safety

Use `do { ... } while (0)` to wrap macros which expand to statements.

```c
// good
#define FAIL(code) do { printf("fail\n"); return code; } while (0)

// bad
#define FAIL(code) printf("fail\n"); return code;
#define FAIL(code) printf("fail\n"); return code
#define FAIL(code) { printf("fail\n"); return code; }
```

This ensures the macro call behaves syntactically like a function call, especially when composed with single-statement `if`-s.

### Minimal macros

Avoid long macros containing several statements. If needed (e.g. when the macro includes control flow statements) prefer defining a helper function and calling it from a more minimal macro.

```c
// good
#define FOO(i) do { foo(i); return; } while (0)
void foo(int i) {
    printf("%d\n", i);
    printf("%d\n", i);
}

// bad
#define foo(i)               \
    do {                     \
        printf("%d\n", (i)); \
        printf("%d\n", (i)); \
        return;              \
    } while (0)
```

### X macros

- Prefix list-like macros with `EACH_`.
- Prefer providing the X macros as arguments. This allows for some higher-order macro passing.
- Use `_` (or `UPPER_CASE` names prefixed with `_` if more than one) as the name for the variable macros in X macros.
- Use normal descriptive names for the concrete implementations of the X macros (at the call site).
- `#undef` the concrete implementation macros immediately after usage.

```c
#define EACH_THING(_) \
    _(foo) \
    _(bar) \
    _(baz)

#define ENUM_MEMBER(name) name,
typedef enum { EACH_THING(ENUM_MEMBER) } thing;
#undef ENUM_MEMBER
```

## Documentation

### Comments

Use multi-line comments for documenting declarations:

```c
/*
 * Documentation
 */
void foo(void);
```

and single-line for non-documentation comments:

```c
// This is just a note on why something is done a certain way
(void)foo;
```
