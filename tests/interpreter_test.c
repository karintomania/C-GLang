#ifndef UNITY_BUILD
  #include "../interpreter.c"
  #include "test_util.c"
#endif

int test_run_interpreter(void) {
  Token tokens[MAX_TOKENS];
  int result;
  AST *ast;
  Expression *expr;

  float res;

  {
    printf("\t%s: 1 / 2 - 3\n", "test_run_interpreter");

    result = run_lexer("1 / 2 - 3", tokens);
    ast = run_parser(tokens, result);
    expr = ast[0]->expr;
    res = interpret(expr);

    ASSERT_EQUAL_NUM(-2.5, res);
  }

  {
    printf("\t%s: 1 + 2 * 3\n", "test_run_interpreter");
    result = run_lexer("1 + 2 * 3", tokens);
    ast = run_parser(tokens, result);
    expr = ast[0]->expr;
    res = interpret(expr);

    ASSERT_EQUAL_NUM(7, res);
  }

  {
    printf("\t%s: 1 / 2 - 3\n", "test_run_interpreter");
    result = run_lexer("1 / 2 - 3", tokens);
    ast = run_parser(tokens, result);
    expr = ast[0]->expr;
    res = interpret(expr);

    ASSERT_EQUAL_NUM(-2.5, res);
  }

  {
    printf("\t%s: 1 / (2 - 3)\n", "test_run_interpreter");
    result = run_lexer("1 / (2 - 3)", tokens);
    ast = run_parser(tokens, result);
    expr = ast[0]->expr;
    res = interpret(expr);

    ASSERT_EQUAL_NUM(-1, res);
  }

  {
    printf("\t%s: -1+-2*-3/-1\n", "test_run_interpreter");
    result = run_lexer("-1+-2*-3/-1", tokens);
    ast = run_parser(tokens, result);
    expr = ast[0]->expr;
    res = interpret(expr);

    ASSERT_EQUAL_NUM(-7, res);
  }

  return 1;
}
