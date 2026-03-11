#define UNITY_BUILD

#include <stdio.h>


#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "lexer.c"
#include "parser.c"
#include "interpreter.c"


int main (int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: cglang -e [program]\n");
    return 2;
  }

  char *program = argv[2];

  Token tokens[MAX_TOKENS];

  int token_count = run_lexer(program, tokens);
  AST *ast = run_parser(tokens, token_count);
  float result = interpret(ast);

  printf("%g\n", result);

  return 0;
}
