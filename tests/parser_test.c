#ifndef UNITY_BUILD
  #include "../lexer.c"
  #include "../parser.c"
#endif

/*---------------------
 Parser test
------------------------*/

int test_run_parser(void) {
  Token tokens[MAX_TOKENS];
  int result = run_lexer("1 + -2 / 4 - 8", tokens);

  AST *ast = run_parser(tokens, result);

  assert(ast->type == AST_TYPE_BINARY_OPERATOR);
  assert(ast->astBO.operator == OP_MINUS);
  AST *l = ast->astBO.left;
  AST *r = ast->astBO.right;

  assert(l->type == AST_TYPE_BINARY_OPERATOR);
  assert(l->astBO.operator == OP_PLUS);
  AST *l_l = l->astBO.left;
  AST *l_r = l->astBO.right;

  assert(l_l->type == AST_TYPE_NUMBER);
  assert(l_l->astNum.number == 1);

  assert(l_r->type == AST_TYPE_BINARY_OPERATOR);
  assert(l_r->astBO.operator == OP_DIV);
  AST *l_r_l = l_r->astBO.left;
  AST *l_r_r = l_r->astBO.right;

  assert(l_r_l->type == AST_TYPE_NUMBER);
  assert(l_r_l->astNum.number == -2);

  assert(l_r_r->type == AST_TYPE_NUMBER);
  assert(l_r_r->astNum.number == 4);

  assert(r->type == AST_TYPE_NUMBER);
  assert(r->astNum.number == 8);

  deinitAst(ast);

  result = run_lexer("(1 --2) *-3", tokens);

  ast = run_parser(tokens, result);

  assert(ast->type == AST_TYPE_BINARY_OPERATOR);
  assert(ast->astBO.operator == OP_MULT);
  l = ast->astBO.left;
  r = ast->astBO.right;

  assert(l->type == AST_TYPE_BINARY_OPERATOR);
  assert(l->astBO.operator == OP_MINUS);

  l_l = l->astBO.left;
  l_r = l->astBO.right;

  assert(l_l->type == AST_TYPE_NUMBER);
  assert(l_l->astNum.number == 1);

  assert(l_r->type == AST_TYPE_NUMBER);
  assert(l_r->astNum.number == -2);

  assert(r->type == AST_TYPE_NUMBER);
  assert(r->astNum.number == -3);

  return 1;
}
