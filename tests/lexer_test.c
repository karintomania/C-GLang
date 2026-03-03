#ifndef UNITY_BUILD
  #include "../lexer.c"
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
    printf("n");
    assert(token_equals(want, got));
  }
}

int test_run_lexer(void) {
  Token tokens[MAX_TOKENS];
  char *str = "0 + 1 - 2.1 * -3 / 100";
  int token_count = run_lexer(str, tokens);

  assert(10 == token_count);

  assert_token_equals(&tokens[0], &(Token){.type = TKN_NUMBER, .num = 0});
  assert_token_equals(&tokens[1], &(Token){.type = TKN_PLUS});
  assert_token_equals(&tokens[2], &(Token){.type = TKN_NUMBER, .num = 1});
  assert_token_equals(&tokens[3], &(Token){.type = TKN_MINUS});
  assert_token_equals(&tokens[4], &(Token){.type = TKN_NUMBER, .num = 2.1});
  assert_token_equals(&tokens[5], &(Token){.type = TKN_MULT});
  assert_token_equals(&tokens[6], &(Token){.type = TKN_MINUS});
  assert_token_equals(&tokens[7], &(Token){.type = TKN_NUMBER, .num = 3});
  assert_token_equals(&tokens[8], &(Token){.type = TKN_DIV});
  assert_token_equals(&tokens[9], &(Token){.type = TKN_NUMBER, .num = 100});

  // test minus signs
  str = "-1 - -2.1";
  token_count = run_lexer(str, tokens);

  assert(5 == token_count);

  assert_token_equals(&tokens[0], &(Token){.type = TKN_MINUS});
  assert_token_equals(&tokens[1], &(Token){.type = TKN_NUMBER, .num = 1});
  assert_token_equals(&tokens[2], &(Token){.type = TKN_MINUS});
  assert_token_equals(&tokens[3], &(Token){.type = TKN_MINUS});
  assert_token_equals(&tokens[4], &(Token){.type = TKN_NUMBER, .num = 2.1});

  // test parenthesis
  str = "(1 + 2)";
  token_count = run_lexer(str, tokens);

  assert(5 == token_count);

  assert_token_equals(&tokens[0], &(Token){.type = TKN_LPAREN});
  assert_token_equals(&tokens[1], &(Token){.type = TKN_NUMBER, .num = 1});
  assert_token_equals(&tokens[2], &(Token){.type = TKN_PLUS});
  assert_token_equals(&tokens[3], &(Token){.type = TKN_NUMBER, .num = 2});
  assert_token_equals(&tokens[4], &(Token){.type = TKN_RPAREN});

  return 1;
}
