#ifndef UNITY_BUILD
#include "parser.c"
#endif

#pragma once

float interpret(Expression *ast) {
  if (ast->type == EXPRESSION_TYPE_NUMBER) {
    return ast->expNum.number;
  }

  if (ast->type == EXPRESSION_TYPE_BINARY_OPERATOR) {
    switch (ast->expBO.operator) {
      case OP_PLUS:
        return interpret(ast->expBO.left) + interpret(ast->expBO.right);
      case OP_MINUS:
        return interpret(ast->expBO.left) - interpret(ast->expBO.right);
      case OP_MULT:
        return interpret(ast->expBO.left) * interpret(ast->expBO.right);
      case OP_DIV:
        return interpret(ast->expBO.left) / interpret(ast->expBO.right);
    }
  }

  printf("unknown ast.");
  exit(1);
}
