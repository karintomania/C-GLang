#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.c"

#pragma once

enum OperatorType {
  OP_PLUS,
  OP_MULT,
  OP_DIV,
  OP_MINUS,
};

typedef struct AST AST;

typedef struct {
  enum OperatorType operator;
  AST *left;
  AST *right;
} ASTBinaryOperator;

typedef struct {
  float number;
} ASTNumber;

enum ASTType {
  AST_TYPE_BINARY_OPERATOR,
  AST_TYPE_NUMBER,
};

struct AST {
  enum ASTType type;
  union {
    ASTBinaryOperator astBO;
    ASTNumber astNum;
  };
};

void printAstRecursive(AST *ast, int depth) {
  if (ast->type == AST_TYPE_BINARY_OPERATOR) {
    char *op_str;
    switch (ast->astBO.operator) {
      case OP_PLUS:
        op_str = "+";
        break;
      case OP_MINUS:
        op_str = "-";
        break;
      case OP_MULT:
        op_str = "*";
        break;
      case OP_DIV:
        op_str = "/";
        break;
    }
    printf("%*snumber:%s\n", depth * 2, " ", op_str);

    printAstRecursive(ast->astBO.left, depth+1);
    printAstRecursive(ast->astBO.right, depth+1);
  } else {
    printf("%*snumber:%f\n", depth * 2, " ", ast->astNum.number);
  }
}

void printAst(AST *ast) {
  printAstRecursive(ast, 0);
}


/*---------------------
 Main Parse logic
------------------------*/

int position;
AST *current;

AST *parse_number(Token *tokens);
AST *parse_term2(Token *tokens);

AST *parse_term2(Token *tokens) {
  current = parse_number(tokens);

  AST *ast = malloc(sizeof(AST));

  Token t = tokens[position++];
  *ast = (AST){
    .type = AST_TYPE_BINARY_OPERATOR,
    .astBO = {
      .operator = (t.type == TKN_PLUS) ? OP_PLUS : OP_MINUS,
      .left = current,
      .right = parse_number(tokens),
    },
  };

  current = ast;

  return ast;
}

AST *parse_number(Token *tokens) {
  Token t = tokens[position];
  position++;
  
  AST *ast = malloc(sizeof(AST));

  *ast = (AST){
    .type = AST_TYPE_NUMBER,
    .astNum = {
      .number = 1.0,
    },
  };

  return ast;
}


AST *run_parser(Token *tokens, int token_n) {
  position = 0;
  current = parse_number(tokens);

  while (position < token_n) {
    position++;
  }
  parse_term2(tokens);

  return current;
}

// free all ASTs
void deinitAst(AST *ast);

