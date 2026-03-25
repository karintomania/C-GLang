.PHONY: test run uninstall
CC=clang
CFLAGS=-Wall -Wpedantic -Wextra --std=c11

cglang:
	$(CC) main.c -o cglang $(CFLAGS)

run: cglang
	./cglang -f test.g

debug-compile:
	clang main.c -o cglang $(CFLAGS) -g

test:
	rm -f ./test
	$(CC) test.c -o test $(CFLAGS)
	./test

debug-test:
	rm -f ./test
	$(CC) test.c -o test $(CFLAGS) -g -lm
	lldb ./test


uninstall:
	rm -f ./cglang ./test
