#ifndef UNITY_BUILD
  #include "parser.c"
  #include "stb_ds.h"
#endif

#pragma once


typedef struct {
  char *key;
  Definition *value;
} DefMap;

typedef struct {
  char *key;
  float value;
} Assignment;

float interpret(AST *ast);
float interpretExpression(Expression *ast, DefMap *dm, Assignment *assignment);

float interpret(AST *ast) {
  DefMap *dm = NULL;

  for (size_t i = 0; i < ast->count; i++) {
    Statement *stmt = ast->stmts[i];
    if (stmt->type == STMT_DEF) {
      Definition *def = stmt->def;
      shput(dm, def->name, def);
    }
    if (stmt->type == STMT_EXPR) {
      return interpretExpression(stmt->expr, dm, NULL);
    }
  }

  return 0;
}

float interpretExpression(Expression *expr, DefMap *dm, Assignment *assignment) {
  if (expr->type == EXPRESSION_UNARY_OPERATOR) {
    float operand = interpretExpression(expr->uo.operand, dm, assignment);

    return operand * -1;
  }

  if (expr->type == EXPRESSION_CALL) {
    float value = interpretExpression(expr->call.expr, dm, assignment);

    Definition *d = shget(dm, expr->call.name);
    shput(assignment, d->args, value);

    return interpretExpression(d->body, dm, assignment);
    
  }

  if (expr->type == EXPRESSION_VAR) {
    return shget(assignment, expr->var.name);
  }

  if (expr->type == EXPRESSION_NUMBER) {
    return expr->num.number;
  }

  if (expr->type == EXPRESSION_BINARY_OPERATOR) {
    switch (expr->bo.operator) {
      case OP_PLUS:
        return interpretExpression(expr->bo.left, dm, assignment)
               + interpretExpression(expr->bo.right, dm, assignment);
      case OP_MINUS:
        return interpretExpression(expr->bo.left, dm, assignment)
               - interpretExpression(expr->bo.right, dm, assignment);
      case OP_MULT:
        return interpretExpression(expr->bo.left, dm, assignment)
               * interpretExpression(expr->bo.right, dm, assignment);
      case OP_DIV:
        return interpretExpression(expr->bo.left, dm, assignment)
               / interpretExpression(expr->bo.right, dm, assignment);
    }
  }

  printf("unknown ast.");
  exit(1);
}
