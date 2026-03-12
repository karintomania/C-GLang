#ifndef UNITY_BUILD
  #include "../lexer.c"
  #include "../parser.c"
  #include "test_util.c"
#endif

/*---------------------
 Parser test
------------------------*/

int test_run_parser(void) {
  AST *ast;
  int result;
  char *expected;

  uint16_t written;
  char buf[4000];

  {
    printf("\t%s: parse math expression\n", "test_run_parser");

    Token tokens[MAX_TOKENS];

    result = run_lexer("1 + -2 / 4 - 8", tokens);

    ast = run_parser(tokens, result);

    written = sprint_ast(ast, buf);

    expected = "\
OP:-\n\
  OP:+\n\
    NUM:1\n\
    OP:/\n\
      OP:-u\n\
        NUM:2\n\
      NUM:4\n\
  NUM:8\n";

    ASSERT_EQUAL_NUM(0, (strncmp(buf, expected, written)));

    deinit_ast(ast);
  }

  {
    printf("\t%s: parse minus\n", "test_run_parser");

    Token tokens[MAX_TOKENS];

    result = run_lexer("(1 --2) *-3", tokens);

    ast = run_parser(tokens, result);

    written = sprint_ast(ast, buf);

    expected = "\
OP:*\n\
  OP:-\n\
    NUM:1\n\
    OP:-u\n\
      NUM:2\n\
  OP:-u\n\
    NUM:3\n";

    ASSERT_EQUAL_NUM(0, (strncmp(buf, expected, written)));

    deinit_ast(ast);
  }

 {
    printf("\t%s: parse function call and vars\n", "test_run_parser");

    Token tokens[MAX_TOKENS];

    result = run_lexer("f(x+y)", tokens);

    ast = run_parser(tokens, result);

    written = sprint_ast(ast, buf);

    expected = "\
CALL:f\n\
  OP:+\n\
    VAR:x\n\
    VAR:y\n\
";

    ASSERT_EQUAL_NUM(0, (strncmp(buf, expected, written)));

    deinit_ast(ast);
  }

 {
    printf("\t%s: parse function definition\n", "test_run_parser");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def f(x) := x + x", tokens);

    ast = run_parser(tokens, result);

    written = sprint_ast(ast, buf);

    expected = "\
DEF:f, ARGS:x\n\
  BODY:\n\
    OP:+\n\
      VAR:x\n\
      VAR:x\n\
";

    ASSERT_EQUAL_NUM(0, (strncmp(buf, expected, written)));

    deinit_ast(ast);
  }

 {
    printf("\t%s: parse function def and call\n", "test_run_parser");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def f(x) := x + x; f(2)", tokens);

    ast = run_parser(tokens, result);

    written = sprint_ast(ast, buf);

    expected = "\
DEF:f, ARGS:x\n\
  BODY:\n\
    OP:+\n\
      VAR:x\n\
      VAR:x\n\
CALL:f\n\
  NUM:2\n\
";

    ASSERT_EQUAL_NUM(0, (strncmp(buf, expected, written)));

    deinit_ast(ast);
  }

  return 1;
}
