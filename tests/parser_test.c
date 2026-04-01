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
  int16_t token_count;
  char *expected;

  uint16_t written;
  char buf[4000];

  Token tokens[MAX_TOKENS];
  char err_buf[256]; 
  LexerError lexer_err = lexer_error_init(err_buf);
  ParserError parser_err = parser_error_init(err_buf);

  {
    printf("\t%s: parse math expression\n", "test_parser_happy_path");

    token_count = run_lexer("1 + -2 / 4 - 8", tokens, &lexer_err);

    assert(token_count != LEXER_ERROR);

    ast = run_parser(tokens, token_count, &parser_err);

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

    token_count = run_lexer("(1 --2) *-3", tokens, &lexer_err);

    ast = run_parser(tokens, token_count, &parser_err);

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

    token_count = run_lexer("f(x+y)", tokens, &lexer_err);

    ast = run_parser(tokens, token_count, &parser_err);

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

    token_count = run_lexer("def f(x) := x + x", tokens, &lexer_err);

    ast = run_parser(tokens, token_count, &parser_err);

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
    printf("\t%s: parse multi-args function definition\n", "test_parser_happy_path");

    token_count = run_lexer("def f(x, y) := x + y", tokens, &lexer_err);

    ast = run_parser(tokens, token_count, &parser_err);

    assert(ast != NULL);
    written = sprint_ast(ast, buf);

    expected = "\
DEF:f, ARGS:x, y\n\
  BODY:\n\
    OP:+\n\
      VAR:x\n\
      VAR:y\n\
";

    ASSERT_EQUAL_NUM(0, (strncmp(buf, expected, written)));

    deinit_ast(ast);
  }

 {
    printf("\t%s: parse multi-args function call\n", "test_parser_happy_path");


    token_count = run_lexer("f(2, 1+1)", tokens, &lexer_err);

    ast = run_parser(tokens, token_count, &parser_err);

    assert(ast != NULL);

    written = sprint_ast(ast, buf);

    expected = "\
CALL:f\n\
  NUM:2\n\
  OP:+\n\
    NUM:1\n\
    NUM:1\n\
";

    ASSERT_EQUAL_NUM(0, (strncmp(buf, expected, written)));

    deinit_ast(ast);
  }

 {
    printf("\t%s: parse function def and call\n", "test_parser_happy_path");


    token_count = run_lexer("def f(x) := x + x;\nf(2)", tokens, &lexer_err);

    ast = run_parser(tokens, token_count, &parser_err);

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

  Token tokens[MAX_TOKENS];
  char err_buf[256]; 
  LexerError lexer_err = lexer_error_init(err_buf);
  ParserError parser_err = parser_error_init(err_buf);

  {
    printf("\t%s: unexpected token\n", "test_parser_error");


    result = run_lexer("1 + +", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast == NULL);
    ASSERT_EQUAL_NUM(PARSER_ERR_UNEXPECTED_TOKEN, parser_err.type);
    ASSERT_EQUAL_NUM(4, parser_err.position);
    ASSERT_EQUAL_NUM(0, strcmp("Unexpected token: TKN_PLUS\n", parser_err.message));
  }

  {
    printf("\t%s: expected var\n", "test_parser_error");


    result = run_lexer("def 1", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast == NULL);
    ASSERT_EQUAL_NUM(PARSER_ERR_UNEXPECTED_TOKEN, parser_err.type);
    ASSERT_EQUAL_NUM(4, parser_err.position);
    ASSERT_EQUAL_NUM(0, strcmp("Expected variable, got TKN_NUMBER\n", parser_err.message));
  }

  {
    printf("\t%s: unexpected eos\n", "test_parser_error");


    result = run_lexer("1 + ", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast == NULL);
    ASSERT_EQUAL_NUM(PARSER_ERR_UNEXPECTED_EOS, parser_err.type);
    ASSERT_EQUAL_NUM(2, parser_err.position);
    ASSERT_EQUAL_NUM(0, strcmp("Unexpected EOS\n", parser_err.message));
  }
}

void test_parser_position(void) {
  AST *ast;
  int result;

  Token tokens[MAX_TOKENS];
  char err_buf[256]; 
  LexerError lexer_err = lexer_error_init(err_buf);
  ParserError parser_err = parser_error_init(err_buf);

  {
    printf("\t%s: expression has correct positions\n", "test_parser_position");


    result = run_lexer("1 + -2", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast != NULL);

    Statement *stmt = ast->stmts[0];
    Expression *e = stmt->expr;
    ASSERT_EQUAL_NUM(0, e->position);
    ASSERT_EQUAL_NUM(EXPRESSION_BINARY_OPERATOR, e->type);

    Expression *l = e->bo.left;
    ASSERT_EQUAL_NUM(0, l->position);
    ASSERT_EQUAL_NUM(EXPRESSION_NUMBER, l->type);
    Expression *r = e->bo.right;
    ASSERT_EQUAL_NUM(4, r->position);
    ASSERT_EQUAL_NUM(EXPRESSION_UNARY_OPERATOR, r->type);

    Expression *o = r->uo.operand;
    ASSERT_EQUAL_NUM(5, o->position);
    ASSERT_EQUAL_NUM(EXPRESSION_NUMBER, o->type);
  }

  {
    printf("\t%s: definition has correct position\n", "test_parser_position");


    result = run_lexer("def f(x) := x", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast != NULL);

    Statement *stmt = ast->stmts[0];
    Definition *d = stmt->def;
    ASSERT_EQUAL_NUM(0, d->position);

    Expression *body = d->body;
    ASSERT_EQUAL_NUM(12, body->position);
    ASSERT_EQUAL_NUM(EXPRESSION_VAR, body->type);
  }

  {
    printf("\t%s: call has correct position\n", "test_parser_position");


    result = run_lexer("f (1)", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    assert(ast != NULL);

    Statement *stmt = ast->stmts[0];
    Expression *e = stmt->expr;
    ASSERT_EQUAL_NUM(0, e->position);
    ASSERT_EQUAL_NUM(EXPRESSION_CALL, e->type);

    Expression *args = e->call.args[0];
    ASSERT_EQUAL_NUM(3, args->position);
    ASSERT_EQUAL_NUM(EXPRESSION_NUMBER, args->type);
  }

}

int test_run_parser(void) {
  test_parser_happy_path();
  test_parser_error();
  test_parser_position();
  return 1;
}
