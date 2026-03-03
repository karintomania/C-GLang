#ifndef UNITY_BUILD
  #include "../lexer.c"
  #include "test_util.c"
#endif

/*---------------------
 Lexer test
------------------------*/

void assert_token_equals(Token *want, Token *got) { 
  if (!token_equals(want, got)) {
    printf("want: ");
    token_print(want);
    printf(", got: ");
    token_print(got);
    printf("\n");
    assert(token_equals(want, got));
  }
}

int test_run_lexer(void) {
  Token tokens[MAX_TOKENS];
  char *str = "0 + 1 - 2.1 * -3 / 100";
  int token_count = run_lexer(str, tokens);

  // assert(10 = token_count);

  assert_token_equals(&(Token){.type = TKN_NUMBER, .num = 0},   &tokens[0]);
  assert_token_equals(&(Token){.type = TKN_PLUS},               &tokens[1]);
  assert_token_equals(&(Token){.type = TKN_NUMBER, .num = 1},   &tokens[2]);
  assert_token_equals(&(Token){.type = TKN_MINUS},              &tokens[3]);
  assert_token_equals(&(Token){.type = TKN_NUMBER, .num = 2.1}, &tokens[4]);
  assert_token_equals(&(Token){.type = TKN_MULT},               &tokens[5]);
  assert_token_equals(&(Token){.type = TKN_MINUS},              &tokens[6]);
  assert_token_equals(&(Token){.type = TKN_NUMBER, .num = 3},   &tokens[7]);
  assert_token_equals(&(Token){.type = TKN_DIV},                &tokens[8]);
  assert_token_equals(&(Token){.type = TKN_NUMBER, .num = 100}, &tokens[9]);

  // test minus signs
  str = "-1 - -2.1";
  token_count = run_lexer(str, tokens);

  assert(5 == token_count);

  assert_token_equals(&(Token){.type = TKN_MINUS},              &tokens[0]);
  assert_token_equals(&(Token){.type = TKN_NUMBER, .num = 1},   &tokens[1]);
  assert_token_equals(&(Token){.type = TKN_MINUS},              &tokens[2]);
  assert_token_equals(&(Token){.type = TKN_MINUS},              &tokens[3]);
  assert_token_equals(&(Token){.type = TKN_NUMBER, .num = 2.1}, &tokens[4]);

  // test parenthesis
  str = "(1 + 2)";
  token_count = run_lexer(str, tokens);

  assert(5 == token_count);

  assert_token_equals(&(Token){.type = TKN_LPAREN},           &tokens[0]);
  assert_token_equals(&(Token){.type = TKN_NUMBER, .num = 1}, &tokens[1]);
  assert_token_equals(&(Token){.type = TKN_PLUS},             &tokens[2]);
  assert_token_equals(&(Token){.type = TKN_NUMBER, .num = 2}, &tokens[3]);
  assert_token_equals(&(Token){.type = TKN_RPAREN},           &tokens[4]);

  // test assignment
  str = "def foo_bar(baz) := baz * baz";
  token_count = run_lexer(str, tokens);

  ASSERT_EQUAL_NUM(2, token_count);

  assert_token_equals(&(Token){.type = TKN_DEF},        &tokens[0]);
  assert_token_equals(&(Token){.type = TKN_ASSIGNMENT}, &tokens[1]);

  return 1;
}
