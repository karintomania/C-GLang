#ifndef UNITY_BUILD
  #include "../interpreter.c"
  #include "test_util.c"
  #include "../stb_ds.h"
#endif

int test_run_interpreter(void) {
  Token tokens[MAX_TOKENS];
  int result;
  AST *ast;

  char err_buf[256]; 
  LexerError lexer_err = lexer_error_init(err_buf);
  ParserError parser_err = parser_error_init(err_buf);

  Value res;

  {
    printf("\t%s: 1 / 2 - 3\n", "test_run_interpreter");

    result = run_lexer("1 / 2 - 3", tokens, &lexer_err);
    ast = run_parser(tokens, result, &parser_err);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(-2.5, res.num);
  }

  {
    printf("\t%s: 1 + 2 * 3\n", "test_run_interpreter");
    result = run_lexer("1 + 2 * 3", tokens, &lexer_err);
    ast = run_parser(tokens, result, &parser_err);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(7, res.num);
  }

  {
    printf("\t%s: 1 / 2 - 3\n", "test_run_interpreter");
    result = run_lexer("1 / 2 - 3", tokens, &lexer_err);
    ast = run_parser(tokens, result, &parser_err);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(-2.5, res.num);
  }

  {
    printf("\t%s: 1 / (2 - 3)\n", "test_run_interpreter");
    result = run_lexer("1 / (2 - 3)", tokens, &lexer_err);
    ast = run_parser(tokens, result, &parser_err);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(-1, res.num);
  }

  {
    printf("\t%s: -1+-2*-3/-1\n", "test_run_interpreter");

    result = run_lexer("-1+-2*-3/-1", tokens, &lexer_err);
    ast = run_parser(tokens, result, &parser_err);
   res = interpret(ast);

    ASSERT_EQUAL_NUM(-7, res.num);
  }

  {
    printf("\t%s: function call\n", "test_run_interpreter");

    result = run_lexer("def f(x) := x*3; f(1)", tokens, &lexer_err);
    ast = run_parser(tokens, result, &parser_err);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(3, res.num);
  }

  {
    printf("\t%s: function call 2\n", "test_run_interpreter");

    result = run_lexer("def f(x) := -x+3; def g(x):=x*x; f(g(2))", tokens, &lexer_err);
    ast = run_parser(tokens, result, &parser_err);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(-1, res.num);
  }

  {
    printf("\t%s: function call 3\n", "test_run_interpreter");

    result = run_lexer("def triple(x) := x*3; def f(x):=-triple(x); -f(3);", tokens, &lexer_err);
    ast = run_parser(tokens, result, &parser_err);
    res = interpret(ast);

    ASSERT_EQUAL_NUM(9, res.num);
  }

  {
    printf("\t%s: return func\n", "test_run_interpreter");

    result = run_lexer("def f(x) := x*3; f;", tokens, &lexer_err);
    ast = run_parser(tokens, result, &parser_err);
    res = interpret(ast);

    assert(VAL_FUNC == res.type);
    ASSERT_EQUAL_NUM(0, strcmp("f", res.func.name));
  }

  return 1;
}
