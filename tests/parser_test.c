#ifndef UNITY_BUILD
  #include "../lexer.c"
  #include "../parser.c"
  #include "test_util.c"
#endif

/*---------------------
 Parser test
------------------------*/

void test_parser_happy_path(void) {
  AST *ast;
  int result;
  char *expected;

  uint16_t written;
  char buf[4000];

  char err_buf[256]; 
  LexerError lexer_err = lexer_error_init(err_buf);
  ParserError parser_err = parser_error_init(err_buf);

  {
    printf("\t%s: parse math expression\n", "test_parser_happy_path");

    Token tokens[MAX_TOKENS];

    result = run_lexer("1 + -2 / 4 - 8", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast != NULL);

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
    printf("\t%s: parse minus\n", "test_parser_happy_path");

    Token tokens[MAX_TOKENS];

    result = run_lexer("(1 --2) *-3", tokens, &lexer_err);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast != NULL);

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
    printf("\t%s: parse function call and vars\n", "test_parser_happy_path");

    Token tokens[MAX_TOKENS];

    result = run_lexer("f(x+y)", tokens, &lexer_err);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast != NULL);

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
    printf("\t%s: parse function definition\n", "test_parser_happy_path");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def f(x) := x + x", tokens, &lexer_err);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast != NULL);

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
    printf("\t%s: parse function def and call\n", "test_parser_happy_path");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def f(x) := x + x;\nf(2)", tokens, &lexer_err);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast != NULL);

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
}

void test_parser_error(void){
  AST *ast;
  int result;

  char err_buf[256]; 
  LexerError lexer_err = lexer_error_init(err_buf);
  ParserError parser_err = parser_error_init(err_buf);

  {
    printf("\t%s: unexpected token\n", "test_parser_error");

    Token tokens[MAX_TOKENS];

    result = run_lexer("1 + +", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast == NULL);
    ASSERT_EQUAL_NUM(PARSER_ERR_UNEXPECTED_TOKEN, parser_err.type);
    ASSERT_EQUAL_NUM(0, strcmp("Unexpected token: TKN_PLUS\n", parser_err.message));
  }

  {
    printf("\t%s: expected var\n", "test_parser_error");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def 1", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast == NULL);
    ASSERT_EQUAL_NUM(PARSER_ERR_UNEXPECTED_TOKEN, parser_err.type);
    ASSERT_EQUAL_NUM(0, strcmp("Expected variable, got TKN_NUMBER\n", parser_err.message));
  }

  {
    printf("\t%s: unexpected eos\n", "test_parser_error");

    Token tokens[MAX_TOKENS];

    result = run_lexer("1 + ", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    printf("%s\n", parser_err.message);

    assert(ast == NULL);
    ASSERT_EQUAL_NUM(PARSER_ERR_UNEXPECTED_EOS, parser_err.type);
    ASSERT_EQUAL_NUM(0, strcmp("Unexpected EOS\n", parser_err.message));
  }
}

int test_run_parser(void) {
  test_parser_happy_path();
  test_parser_error();
  return 1;
}
