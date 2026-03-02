#ifndef UNITY_BUILD
#include "parser.c"
#endif

#pragma once

float interpret(AST *ast) {
  if (ast->type == AST_TYPE_NUMBER) {
    return ast->astNum.number;
  }

  if (ast->type == AST_TYPE_BINARY_OPERATOR) {
    switch (ast->astBO.operator) {
      case OP_PLUS:
        return interpret(ast->astBO.left) + interpret(ast->astBO.right);
      case OP_MINUS:
        return interpret(ast->astBO.left) - interpret(ast->astBO.right);
      case OP_MULT:
        return interpret(ast->astBO.left) * interpret(ast->astBO.right);
      case OP_DIV:
        return interpret(ast->astBO.left) / interpret(ast->astBO.right);
    }
  }

  printf("unknown ast.");
  exit(1);
}
