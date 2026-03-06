#ifndef UNITY_BUILD
  #include "../lexer.c"
  #include "../parser.c"
  #include "test_util.c"
#endif

/*---------------------
 Parser test
------------------------*/

int test_run_parser(void) {
  Token tokens[MAX_TOKENS];
  Expression *ast;
  int result;
  char *expected;

  uint16_t written;
  char buf[4000];

  result = run_lexer("1 + -2 / 4 - 8", tokens);

  ast = run_parser(tokens, result);

  written = sprint_ast(ast, buf);

  expected = "\
OP:-\n\
  OP:+\n\
    NUM:1\n\
    OP:/\n\
      NUM:-2\n\
      NUM:4\n\
  NUM:8\n";

  ASSERT_EQUAL_NUM(0, (strncmp(buf, expected, written)));

  deinit_ast(ast);

  result = run_lexer("(1 --2) *-3", tokens);

  ast = run_parser(tokens, result);

  written = sprint_ast(ast, buf);

  expected = "\
OP:*\n\
  OP:-\n\
    NUM:1\n\
    NUM:-2\n\
  NUM:-3\n";

  ASSERT_EQUAL_NUM(0, (strncmp(buf, expected, written)));

  deinit_ast(ast);

  result = run_lexer("f", tokens);

  ast = run_parser(tokens, result);

  written = sprint_ast(ast, buf);

  expected = "VAR:f\n";

  ASSERT_EQUAL_NUM(0, (strncmp(buf, expected, written)));

  deinit_ast(ast);

  return 1;
}
