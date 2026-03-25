#ifndef UNITY_BUILD
  #include "../lexer.c"
  #include "../parser.c"
  #include "../typechecker.c"
  #include "test_util.c"
#endif

void test_typechecker_unify(void) {
  Type *want, *got, *result;

  char buf[512];
  TypeError err = type_error_init(buf);

  {
    printf("\t%s: unify unknown\n", "test_typechecker_unify");

    want = &(Type){.type = TYPE_UNKNOWN};
    got = &(Type){.type = TYPE_NUM};

    result = unify(want, got, &err);

    ASSERT_EQUAL_NUM(TYPE_NUM, result->type);

    want = &(Type){.type = TYPE_NUM};
    got = &(Type){.type = TYPE_UNKNOWN};

    result = unify(want, got, &err);

    ASSERT_EQUAL_NUM(TYPE_NUM, result->type);
  }

  {
    printf("\t%s: unify number\n", "test_typechecker_unify");

    want = &(Type){.type = TYPE_NUM};
    got = &(Type){.type = TYPE_NUM};

    result = unify(want, got, &err);

    ASSERT_EQUAL_NUM(TYPE_NUM, result->type);
  }

  {
    printf("\t%s: unify function\n", "test_typechecker_unify");

    Type *arg = &(Type){.type = TYPE_NUM};
    Type *result = &(Type){.type = TYPE_NUM};
    want = &(Type){.type = TYPE_FUNC, .args = &(arg), .arg_len = 1, .result = result};
    got = &(Type){.type = TYPE_FUNC, .args = &(arg), .arg_len = 1, .result = result};

    result = unify(want, got, &err);

    ASSERT_EQUAL_NUM(TYPE_FUNC, result->type);
    ASSERT_EQUAL_NUM(1, result->arg_len);
    ASSERT_EQUAL_NUM(TYPE_NUM, result->args[0]->type);
  }

  {
    printf("\t%s: mismatch &error\n", "test_typechecker_unify");

    Type *arg = &(Type){.type = TYPE_NUM};
    Type *result = &(Type){.type = TYPE_NUM};

    want = &(Type){.type = TYPE_FUNC, .args = &(arg), .arg_len = 1, .result = result};
    got = &(Type){.type = TYPE_NUM};

    result = unify(want, got, &err);

    assert(NULL == result);
    ASSERT_EQUAL_NUM(TYPE_ERR_TYPE_MISMATCH, err.err_type);
    assert(want == err.mm.want);
    assert(got == err.mm.got);
  }
}

void test_typechecker_happy_path(void) {
  AST *ast;
  int result;

  char err_buf[256]; 
  LexerError lexer_err = lexer_error_init(err_buf);
  ParserError parser_err = parser_error_init(err_buf);
  TypeError type_err = type_error_init(err_buf);

  {
    printf("\t%s: parse math expression\n", "test_typechecker_happy_path");

    Token tokens[MAX_TOKENS];

    result = run_lexer("1 + -2 / 4 - 8", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    Type *res = run_typechecker(ast, &type_err);

    ASSERT_EQUAL_NUM(TYPE_NUM, res->type);
  }

  {
    printf("\t%s: parse def\n", "test_typechecker_happy_path");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def f(x) := x * x;f", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    Type *res = run_typechecker(ast, &type_err);

    ASSERT_EQUAL_NUM(TYPE_FUNC, res->type);
    ASSERT_EQUAL_NUM(1, res->arg_len);
    assert(&type_number == res->args[0]);
  }

  {
    printf("\t%s: check nested functions\n", "test_typechecker_happy_path");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def f(x) := -x+3; def g(x):=x*x; f(g(2))", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    Type *res = run_typechecker(ast, &type_err);

    ASSERT_EQUAL_NUM(TYPE_NUM, res->type);
  }

}

void test_typechecker_error(void){
  AST *ast;
  int result;

  char err_buf[256]; 
  LexerError lexer_err = lexer_error_init(err_buf);
  ParserError parser_err = parser_error_init(err_buf);
  TypeError type_err = type_error_init(err_buf);

  {
    printf("\t%s: type mismatch\n", "test_typechecker_error");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def f(x) := x; f(f);", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    Type *res = run_typechecker(ast, &type_err);

    assert(NULL == res);
    ASSERT_EQUAL_NUM(type_err.err_type, TYPE_ERR_TYPE_MISMATCH);
  }

  {
    printf("\t%s: type mismatch 2\n", "test_typechecker_error");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def f(x) := x;1 + f", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    Type *res = run_typechecker(ast, &type_err);

    assert(NULL == res);
    ASSERT_EQUAL_NUM(type_err.err_type, TYPE_ERR_TYPE_MISMATCH);
  }

  {
    printf("\t%s: undefined var\n", "test_typechecker_error");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def f(x) := y; f(1)", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    Type *res = run_typechecker(ast, &type_err);

    assert(NULL == res);
    ASSERT_EQUAL_NUM(type_err.err_type, TYPE_ERR_UNDEFINED_VAR);
  }

  {
    printf("\t%s: undefined var 2\n", "test_typechecker_error");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def f(x) := x; f(x)", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    Type *res = run_typechecker(ast, &type_err);

    assert(NULL == res);
    ASSERT_EQUAL_NUM(type_err.err_type, TYPE_ERR_UNDEFINED_VAR);
  }

  {
    printf("\t%s: undefined func\n", "test_typechecker_error");

    Token tokens[MAX_TOKENS];

    result = run_lexer("def f(x) := x; g(1)", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    Type *res = run_typechecker(ast, &type_err);

    assert(NULL == res);
    ASSERT_EQUAL_NUM(type_err.err_type, TYPE_ERR_UNDEFINED_FUNC);
  }

  {
    printf("\t%s: expr not allowed\n", "test_typechecker_error");

    Token tokens[MAX_TOKENS];

    result = run_lexer("1+3;1;", tokens, &lexer_err);

    assert(result != LEXER_ERROR);

    ast = run_parser(tokens, result, &parser_err);

    Type *res = run_typechecker(ast, &type_err);

    assert(NULL == res);
    ASSERT_EQUAL_NUM(type_err.err_type, TYPE_ERR_EXPR_NOT_ALLOWED);
  }
}

int test_run_typechecker(void) {
  test_typechecker_unify();
  test_typechecker_happy_path();
  test_typechecker_error();
  return 1;
}
