CC = clang
CFLAGS = -O3 -std=c99 -Wall -Wextra -Wpedantic -Wno-char-subscripts \
	-Wno-parentheses

bin/hammer: lib/*.c lib/*.h src/*.c src/*.h
	mkdir -p bin
	$(CC) lib/*.c src/*.c -o $@ $(CFLAGS)

bin/build_test: build_test.c test/*.c
	mkdir -p bin
	$(CC) $< -o $@ $(CFLAGS)

bin/test: bin/build_test lib/*.c lib/*.h test/*.c test/*.h
	bin/build_test test/tests.gen.h test/*.c
	$(CC) lib/*.c test/*.c -o $@ $(CFLAGS)

.PHONY: test
test: bin/test
	bin/test

.PHONY: format
format:
	clang-format -i **/*.{c,h}
