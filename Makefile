CC = clang
CFLAGS = -O3 -std=c99 -Wall -Wextra -Wpedantic -Wno-char-subscripts

bin/hammer: lib/*.c lib/*.h src/*.c src/*.h
	mkdir -p bin
	$(CC) lib/*.c src/*.c -o $@ $(CFLAGS)

bin/test: lib/*.c lib/*.h test/*.c test/*.h
	mkdir -p bin
	$(CC) lib/*.c test/*.c -o $@ $(CFLAGS)

.PHONY: test
test: bin/test
	bin/test

.PHONY: format
format:
	clang-format -i **/*.{c,h}
