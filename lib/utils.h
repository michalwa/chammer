#ifndef UTILS_H_
#define UTILS_H_

#define ARGC(...)                                         ARGC_(__VA_ARGS__, ARGC_SEQ_)
#define ARGC_(...)                                        ARGC_NTH_(__VA_ARGS__)
#define ARGC_NTH_(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define ARGC_SEQ_                                         8, 7, 6, 5, 4, 3, 2, 1, 0

#endif // UTILS_H_
