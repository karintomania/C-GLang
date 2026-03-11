#ifndef UNITY_BUILD
#include "parser.c"
#endif

#pragma once

float interpret(Expression *ast) {
  if (ast->type == EXPRESSION_NUMBER) {
    return ast->num.number;
  }

  if (ast->type == EXPRESSION_BINARY_OPERATOR) {
    switch (ast->bo.operator) {
      case OP_PLUS:
        return interpret(ast->bo.left) + interpret(ast->bo.right);
      case OP_MINUS:
        return interpret(ast->bo.left) - interpret(ast->bo.right);
      case OP_MULT:
        return interpret(ast->bo.left) * interpret(ast->bo.right);
      case OP_DIV:
        return interpret(ast->bo.left) / interpret(ast->bo.right);
    }
  }

  printf("unknown ast.");
  exit(1);
}
