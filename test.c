#include "cglang.c"
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#define MAX_TESTS 1000

typedef struct {
    int (*ptr)(void);
    const char *name;
} TestFunc;

#define FUNC_DEF(func) {func, #func}

int test_run_lexer(void) {
  Token tokens[MAX_TOKENS];
  char *str = "1+2.1";
  run_lexer(str, tokens);

  assert(tokens[0].type == NUMBER);
  assert(tokens[0].num == 1);

  assert(tokens[1].type == PLUS);

  assert(tokens[2].type == NUMBER);

  printf("%.2f\n", tokens[2].num);
  assert((tokens[2].num - 2.1) == 0);

  return 1;
}

int main(void) {
  TestFunc tests[] = {
    FUNC_DEF(test_run_lexer),
    {NULL, ""},
  };

  for (int i=0; i < MAX_TESTS; i++) {
    TestFunc test = tests[i];

    if (test.ptr != NULL) {
      test.ptr();
    } else {
      break;
    }
  }
  printf("All tests passed.\n");
  return 1;
}

