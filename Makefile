CFLAGS = -O3 -std=c99 -Wall -Wextra -Wpedantic -Wno-char-subscripts

bin/hammer: src/*.c src/*.h
	mkdir -p bin
	gcc src/*.c -o bin/hammer $(CFLAGS)

.PHONY: format
format:
	clang-format -i src/*.c src/*.h
