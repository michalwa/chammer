#ifndef UTILS_H_
#define UTILS_H_

#define HAMMER_DEBUG_ASSERTION_FAILURE 100

#define ARGC(...)                                         ARGC_(__VA_ARGS__, ARGC_SEQ_)
#define ARGC_(...)                                        ARGC_NTH_(__VA_ARGS__)
#define ARGC_NTH_(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define ARGC_SEQ_                                         8, 7, 6, 5, 4, 3, 2, 1, 0

/*
 * Shorthand wrapper for safely defining macros which expand to statements
 */
#define DO(body)           \
    do { body; } while (0)

#define RETURN_ENUM_NAME(type, value, x_macro)               \
    do {                                                     \
        type value_ = (value);                               \
        switch (value_) {                                    \
            x_macro(RETURN_ENUM_NAME_CASE_);                 \
        default: panic("invalid " #type ": %d", (int)value); \
        }                                                    \
    } while (0)

#define RETURN_ENUM_NAME_CASE_(name) \
    case name: return #name;

#define panic(fmt, ...) panic_(fmt, __FILE__, __LINE__, __VA_ARGS__)

void panic_(const char *fmt, const char *file, int line, ...);

#endif // UTILS_H_
