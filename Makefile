.PHONY: test run uninstall

CFLAGS=-Wall -Wpedantic -Wextra --std=c99

cglang:
	clang main.c -o cglang $(CFLAGS)

run: cglang
	./cglang

test:
	rm -f ./test
	clang test.c -o test $(CFLAGS)
	./test

debug-test:
	rm -f ./test
	clang test.c -o test $(CFLAGS) -g -lm
	lldb ./test


uninstall:
	rm -f ./cglang ./test
