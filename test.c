#include "cglang.c"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*---------------------
 Lexer test
------------------------*/
int test_run_lexer(void) {
  Token tokens[MAX_TOKENS];
  char *str = "1 - 2.1 * -3 / 100";
  int token_count = run_lexer(str, tokens);

  assert(7 == token_count);

  assert(Token_equals(&tokens[0], &(Token){.type = NUMBER, .num = 1}));
  assert(Token_equals(&tokens[1], &(Token){.type = MINUS}));
  assert(Token_equals(&tokens[2], &(Token){.type = NUMBER, .num = 2.1}));
  assert(Token_equals(&tokens[3], &(Token){.type = MULT}));
  assert(Token_equals(&tokens[4], &(Token){.type = NUMBER, .num = -3}));
  assert(Token_equals(&tokens[5], &(Token){.type = DIV}));
  assert(Token_equals(&tokens[6], &(Token){.type = NUMBER, .num = 100}));

  // test minus signs
  str = "-1 - -2.1";
  token_count = run_lexer(str, tokens);

  assert(3 == token_count);

  assert(Token_equals(&tokens[0], &(Token){.type = NUMBER, .num = -1}));
  assert(Token_equals(&tokens[1], &(Token){.type = MINUS}));
  assert(Token_equals(&tokens[2], &(Token){.type = NUMBER, .num = -2.1}));

  return 1;
}

// Testing framework
#define MAX_TESTS 1000

typedef struct {
    int (*ptr)(void);
    const char *name;
} TestFunc;

#define FUNC_DEF(func) {func, #func}

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

