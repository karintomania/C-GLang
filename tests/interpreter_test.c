#ifndef UNITY_BUILD
  #include "../interpreter.c"
  #include "test_util.c"
  #include "../stb_ds.h"
#endif

int test_run_interpreter(void) {
  Token tokens[MAX_TOKENS];
  int result;
  AST *ast;

  float res;

  {
    printf("\t%s: 1 / 2 - 3\n", "test_run_interpreter");

    result = run_lexer("1 / 2 - 3", tokens);
    ast = run_parser(tokens, result);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(-2.5, res);
  }

  {
    printf("\t%s: 1 + 2 * 3\n", "test_run_interpreter");
    result = run_lexer("1 + 2 * 3", tokens);
    ast = run_parser(tokens, result);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(7, res);
  }

  {
    printf("\t%s: 1 / 2 - 3\n", "test_run_interpreter");
    result = run_lexer("1 / 2 - 3", tokens);
    ast = run_parser(tokens, result);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(-2.5, res);
  }

  {
    printf("\t%s: 1 / (2 - 3)\n", "test_run_interpreter");
    result = run_lexer("1 / (2 - 3)", tokens);
    ast = run_parser(tokens, result);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(-1, res);
  }

  {
    printf("\t%s: -1+-2*-3/-1\n", "test_run_interpreter");

    result = run_lexer("-1+-2*-3/-1", tokens);
    ast = run_parser(tokens, result);
   res = interpret(ast);

    ASSERT_EQUAL_NUM(-7, res);
  }

  {
    printf("\t%s: function call\n", "test_run_interpreter");

    result = run_lexer("def f(x) := x*3; f(1)", tokens);
    ast = run_parser(tokens, result);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(3, res);
  }

  {
    printf("\t%s: function call 2\n", "test_run_interpreter");

    result = run_lexer("def f(x) := -x+3; def g(x):=x*x; f(g(2))", tokens);
    ast = run_parser(tokens, result);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(-1, res);
  }

  {
    printf("\t%s: function call 3\n", "test_run_interpreter");

    result = run_lexer("def triple(x) := x*3; def f(x):=-triple(x); -f(3);", tokens);
    ast = run_parser(tokens, result);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(9, res);
  }

  return 1;
}
