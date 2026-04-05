#ifndef UNITY_BUILD
  #include "../interpreter.c"
  #include "../typechecker.c"
  #include "test_util.c"
  #include "../stb_ds.h"
#endif

int test_run_interpreter(void) {
  Token tokens[MAX_TOKENS];
  int token_count;
  AST *ast;

  char err_buf[256]; 
  LexerError lexer_err = lexer_error_init(err_buf);
  ParserError parser_err = parser_error_init(err_buf);
  TypeError type_err = type_error_init(err_buf);

  InterpretResult interpret_res;

  {
    printf("\t%s: 1 / 2 - 3\n", "test_run_interpreter");

    token_count = run_lexer("1 / 2 - 3", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    ASSERT_EQUAL_NUM(-2.5, res.num);
  }

  {
    printf("\t%s: 1 + 2 * 3\n", "test_run_interpreter");
    token_count = run_lexer("1 + 2 * 3", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    ASSERT_EQUAL_NUM(7, res.num);
  }

  {
    printf("\t%s: 1 / 2 - 3\n", "test_run_interpreter");
    token_count = run_lexer("1 / 2 - 3", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    ASSERT_EQUAL_NUM(-2.5, res.num);
  }

  {
    printf("\t%s: 1 / (2 - 3)\n", "test_run_interpreter");
    token_count = run_lexer("1 / (2 - 3)", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    ASSERT_EQUAL_NUM(-1, res.num);
  }

  {
    printf("\t%s: -1+-2*-3/-1\n", "test_run_interpreter");

    token_count = run_lexer("-1+-2*-3/-1", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
   interpret_res = interpret(ast);
   Value res = interpret_res.v;

    ASSERT_EQUAL_NUM(-7, res.num);
  }

  {
    printf("\t%s: function call\n", "test_run_interpreter");

    token_count = run_lexer("def f(x) := x*3; f(1)", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    ASSERT_EQUAL_NUM(3, res.num);
  }

  {
    printf("\t%s: function call 2\n", "test_run_interpreter");

    token_count = run_lexer("def f(x) := -x+3; def g(x):=x*x; f(g(2))", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    ASSERT_EQUAL_NUM(-1, res.num);
  }

  {
    printf("\t%s: function call 3\n", "test_run_interpreter");

    token_count = run_lexer("def triple(x) := x*3; def f(x):=-triple(x); -f(3);", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    ASSERT_EQUAL_NUM(9, res.num);
  }

  {
    printf("\t%s: return func\n", "test_run_interpreter");

    token_count = run_lexer("def f(x) := x*3; f;", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    assert(VAL_FUNC == res.type);
    ASSERT_EQUAL_NUM(0, strcmp("f", res.func.name));
  }

  {
    printf("\t%s: interpret builtin\n", "test_run_interpreter");

    token_count = run_lexer("sin(0);", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    assert(VAL_NUM == res.type);
    ASSERT_EQUAL_NUM(0, res.num);
  }

  {
    printf("\t%s: interpret multi-args func\n", "test_run_interpreter");

    token_count = run_lexer("def f(x, y) := x + y;f(1, 3);", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    assert(VAL_NUM == res.type);
    ASSERT_EQUAL_NUM(4, res.num);
  }

  {
    printf("\t%s: interpret multi-args builtin - max\n", "test_run_interpreter");

    token_count = run_lexer("max(1, 3);", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    assert(VAL_NUM == res.type);
    ASSERT_EQUAL_NUM(3, res.num);
  }

  {
    printf("\t%s: interpret multi-args builtin - max 2\n", "test_run_interpreter");

    token_count = run_lexer("max(4.1, -1);", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    assert(VAL_NUM == res.type);
    ASSERT_EQUAL_NUM(4.1, res.num);
  }

  {
    printf("\t%s: interpret conditional - then\n", "test_run_interpreter");

    token_count = run_lexer("def f(x) := x ? x * 2 : -x * 3; f(2);", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    assert(VAL_NUM == res.type);
    ASSERT_EQUAL_NUM(4, res.num);
  }

  {
    printf("\t%s: interpret conditional - else\n", "test_run_interpreter");

    token_count = run_lexer("def f(x) := x ? x * 2 : -x * 3; f(-2);", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    assert(VAL_NUM == res.type);
    ASSERT_EQUAL_NUM(6, res.num);
  }

  {
    printf("\t%s: interpret recursive\n", "test_run_interpreter");

    token_count = run_lexer("def f(x) := x ? x + f(x-1) : x; f(5);", tokens, &lexer_err);
    ast = run_parser(tokens, token_count, &parser_err);
    assert(NULL != run_typechecker(ast, &type_err));
    interpret_res = interpret(ast);
    Value res = interpret_res.v;

    assert(VAL_NUM == res.type);
    ASSERT_EQUAL_NUM(15, res.num);
  }
  return 1;
}
