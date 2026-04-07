# C-GLang

C-GLang is a C implementation of the GLang interpreter based on this tutorial: https://zenn.dev/myuon/books/a5f70d97dccd7c

# Getting Started
Run `make cglang` to build the executable `cglang`.

You can pass a program with the `-e` option, or a file path with the `-f` option.
```
$ make cglang
// builds the cglang executable

$ ./cglang -e "1+1"
// prints 2

$ ./cglang -f test.g
// prints the result of test.g
```

To use the graph plotting feature, `gnuplot` must be available on your system.

# Features
## Mathematical Arithmetic
C-GLang supports the basic arithmetic operators `+`, `-`, `*`, and `/`.
```
$ ./cglang -e "1+1"
// prints 2
```

## Functions
You can define functions using the `def` keyword and the definition operator `:=`.
```
$ ./cglang -e "def f(x) := x * 3; f(10);"
// prints 30

// Multiple arguments are supported as well
$ ./cglang -e "def add(x, y) := x + y; add(5, 6);"
// prints 11
```

## Built-in Functions
The following built-in functions are available:
- sin(x)
- cos(x)
- sqrt(x)
- min(x, y)
- max(x, y)

```
$ ./cglang -e "cos(0);"
// prints 1
```

## Conditional Operator
The conditional operator `?` returns either the "then" or "else" expression depending on whether the condition is positive or negative.
```
$ ./cglang -e "def abs(x) := x ? x : -x; abs(10);"
// prints 10

$ ./cglang -e "def abs(x) := x ? x : -x; abs(-100);"
// prints 100
```

## Graph Plotting
If the program returns a function, C-GLang plots its graph. Only single-argument functions are supported.

```
// plot a square wave approximation
./cglang -e "def f(n, x) := n ? (4 / 3.14159265) * sin((2 * n - 1) * x) / (2 * n - 1) + f(n - 1, x) : 0; \
def g(x) := f(1000,x); \
g;"
```
