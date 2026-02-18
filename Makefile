.PHONY: test run uninstall

cglang:
	clang main.c -o cglang -Wall -Wpedantic --std=c99

run: cglang
	./cglang

test:
	rm -f ./test
	clang test.c -o test -Wall -Wpedantic --std=c99
	./test

uninstall:
	rm -f ./cglang ./test
