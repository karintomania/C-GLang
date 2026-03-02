#define UNITY_BUILD

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "lexer.c"
#include "parser.c"
#include "interpreter.c"

/*---------------------
 Lexer test
------------------------*/
int test_run_lexer(void) {
  Token tokens[MAX_TOKENS];
  char *str = "0 + 1 - 2.1 * -3 / 100";
  int token_count = run_lexer(str, tokens);

  assert(9 == token_count);

  assert(token_equals(&tokens[0], &(Token){.type = TKN_NUMBER, .num = 0}));
  assert(token_equals(&tokens[1], &(Token){.type = TKN_PLUS}));
  assert(token_equals(&tokens[2], &(Token){.type = TKN_NUMBER, .num = 1}));
  assert(token_equals(&tokens[3], &(Token){.type = TKN_MINUS}));
  assert(token_equals(&tokens[4], &(Token){.type = TKN_NUMBER, .num = 2.1}));
  assert(token_equals(&tokens[5], &(Token){.type = TKN_MULT}));
  assert(token_equals(&tokens[6], &(Token){.type = TKN_NUMBER, .num = -3}));
  assert(token_equals(&tokens[7], &(Token){.type = TKN_DIV}));
  assert(token_equals(&tokens[8], &(Token){.type = TKN_NUMBER, .num = 100}));

  // test minus signs
  str = "-1 - -2.1";
  token_count = run_lexer(str, tokens);

  assert(3 == token_count);

  assert(token_equals(&tokens[0], &(Token){.type = TKN_NUMBER, .num = -1}));
  assert(token_equals(&tokens[1], &(Token){.type = TKN_MINUS}));
  assert(token_equals(&tokens[2], &(Token){.type = TKN_NUMBER, .num = -2.1}));

  // test parenthesis
  str = "(1 + 2)";
  token_count = run_lexer(str, tokens);

  assert(5 == token_count);

  assert(token_equals(&tokens[0], &(Token){.type = TKN_LPAREN}));
  assert(token_equals(&tokens[1], &(Token){.type = TKN_NUMBER, .num = 1}));
  assert(token_equals(&tokens[2], &(Token){.type = TKN_PLUS}));
  assert(token_equals(&tokens[3], &(Token){.type = TKN_NUMBER, .num = 2}));
  assert(token_equals(&tokens[4], &(Token){.type = TKN_RPAREN}));

  return 1;
}

/*---------------------
 Parser test
------------------------*/

int test_run_parser(void) {
  Token tokens[MAX_TOKENS];
  int result = run_lexer("1 + 2 / 4 - 8", tokens);

  AST *ast = run_parser(tokens, result);

  assert(ast->type == AST_TYPE_BINARY_OPERATOR);
  assert(ast->astBO.operator == OP_MINUS);
  AST *l = ast->astBO.left;
  AST *r = ast->astBO.right;

  assert(l->type == AST_TYPE_BINARY_OPERATOR);
  assert(l->astBO.operator == OP_PLUS);
  AST *l_l = l->astBO.left;
  AST *l_r = l->astBO.right;

  assert(l_l->type == AST_TYPE_NUMBER);
  assert(l_l->astNum.number == 1);

  assert(l_r->type == AST_TYPE_BINARY_OPERATOR);
  assert(l_r->astBO.operator == OP_DIV);
  AST *l_r_l = l_r->astBO.left;
  AST *l_r_r = l_r->astBO.right;

  assert(l_r_l->type == AST_TYPE_NUMBER);
  assert(l_r_l->astNum.number == 2);

  assert(l_r_r->type == AST_TYPE_NUMBER);
  assert(l_r_r->astNum.number == 4);

  assert(r->type == AST_TYPE_NUMBER);
  assert(r->astNum.number == 8);

  deinitAst(ast);

  result = run_lexer("(1 + 2) * 3", tokens);

  ast = run_parser(tokens, result);

  return 1;
}

/*---------------------
 Interpreter test
------------------------*/

int test_run_interpreter(void) {
  Token tokens[MAX_TOKENS];
  int result = run_lexer("1 + 2", tokens);
  AST *ast = run_parser(tokens, result);
  float res = interpret(ast);

  assert(fabs(res - 3) < 0.1);

  result = run_lexer("1 + 2 * 3", tokens);
  ast = run_parser(tokens, result);
  res = interpret(ast);

  assert(fabs(res - 7) < 0.1);

  result = run_lexer("1 / 2 - 3", tokens);
  ast = run_parser(tokens, result);
  res = interpret(ast);

  assert(fabs(res - -2.5) < 0.1);

  result = run_lexer("1 / (2 - 3)", tokens);
  ast = run_parser(tokens, result);
  res = interpret(ast);

  assert(fabs(res - -1) < 0.1);

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
    FUNC_DEF(test_run_interpreter),
    {NULL, ""},
  };

  for (int i=0; i < MAX_TESTS; i++) {
    TestFunc test = tests[i];

    if (test.ptr != NULL) {
      test.ptr();
      printf("%s passed!\n", test.name);
    } else {
      break;
    }
  }
  printf("All tests passed.\n");
  return 0;
}

