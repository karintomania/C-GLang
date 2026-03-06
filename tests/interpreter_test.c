#ifndef UNITY_BUILD
  #include "../interpreter.c"
  #include "test_util.c"
#endif

int test_run_interpreter(void) {
  Token tokens[MAX_TOKENS];
  int result = run_lexer("1 + 2", tokens);
  Expression *ast = run_parser(tokens, result);
  float res = interpret(ast);

  ASSERT_EQUAL_NUM(3, res);

  result = run_lexer("1 + 2 * 3", tokens);
  ast = run_parser(tokens, result);
  res = interpret(ast);

  ASSERT_EQUAL_NUM(7, res);

  result = run_lexer("1 / 2 - 3", tokens);
  ast = run_parser(tokens, result);
  res = interpret(ast);

  ASSERT_EQUAL_NUM(-2.5, res);

  result = run_lexer("1 / (2 - 3)", tokens);
  ast = run_parser(tokens, result);
  res = interpret(ast);

  ASSERT_EQUAL_NUM(-1, res);

  result = run_lexer("-1+-2*-3/-1", tokens);
  ast = run_parser(tokens, result);
  res = interpret(ast);

  ASSERT_EQUAL_NUM(-7, res);

  return 1;
}
