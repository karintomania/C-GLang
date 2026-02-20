#include "lexer.c"
#include "parser.c"
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

  assert(token_equals(&tokens[0], &(Token){.type = TKN_NUMBER, .num = 1}));
  assert(token_equals(&tokens[1], &(Token){.type = TKN_MINUS}));
  assert(token_equals(&tokens[2], &(Token){.type = TKN_NUMBER, .num = 2.1}));
  assert(token_equals(&tokens[3], &(Token){.type = TKN_MULT}));
  assert(token_equals(&tokens[4], &(Token){.type = TKN_NUMBER, .num = -3}));
  assert(token_equals(&tokens[5], &(Token){.type = TKN_DIV}));
  assert(token_equals(&tokens[6], &(Token){.type = TKN_NUMBER, .num = 100}));

  // test minus signs
  str = "-1 - -2.1";
  token_count = run_lexer(str, tokens);

  assert(3 == token_count);

  assert(token_equals(&tokens[0], &(Token){.type = TKN_NUMBER, .num = -1}));
  assert(token_equals(&tokens[1], &(Token){.type = TKN_MINUS}));
  assert(token_equals(&tokens[2], &(Token){.type = TKN_NUMBER, .num = -2.1}));

  return 1;
}

/*---------------------
 Parser test
------------------------*/

int test_run_parser(void) {
  Token tokens[MAX_TOKENS];
  int result = run_lexer("1 + 2", tokens);
  assert(result == 3);

  AST *ast = run_parser(tokens);

  assert(ast->type == AST_TYPE_NUMBER);

  printAst(ast);

  return 1;
}

/*---------------------
 Testing framework
------------------------*/
#define MAX_TESTS 1000

typedef struct {
    int (*ptr)(void);
    const char *name;
} TestFunc;

#define FUNC_DEF(func) {func, #func}

int main(void) {
  TestFunc tests[] = {
    FUNC_DEF(test_run_lexer),
    FUNC_DEF(test_run_parser),
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

