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

void print_ast_recursive(AST *ast, int depth) {
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
    printf("%*sop:%s\n", depth * 2, " ", op_str);

    print_ast_recursive(ast->astBO.left, depth+1);
    print_ast_recursive(ast->astBO.right, depth+1);
  } else {
    printf("%*snum:%f\n", depth * 2, " ", ast->astNum.number);
  }
}

void print_ast(AST *ast) {
  print_ast_recursive(ast, 0);
}

/*---------------------
 Main Parse logic
------------------------*/

int position;
AST *current;
int parser_token_count;

AST *parse_term2(Token *tokens);
AST *parse_term1(Token *tokens);
AST *parse_number(Token *tokens);

AST *parse_term2(Token *tokens) {
  current = parse_term1(tokens);

  while (position < parser_token_count) {
    AST *ast = malloc(sizeof(AST));

    Token t = tokens[position];

    if (t.type != TKN_PLUS && t.type != TKN_MINUS) break;

    position++;
    *ast = (AST){
      .type = AST_TYPE_BINARY_OPERATOR,
      .astBO = {
        .operator = (t.type == TKN_PLUS) ? OP_PLUS : OP_MINUS,
        .left = current,
        .right = parse_term1(tokens),
      },
    };

    current = ast;
  }

  return current;
}

AST *parse_term1(Token *tokens) {
  current = parse_number(tokens);

  while (position < parser_token_count) {
      AST *ast = malloc(sizeof(AST));

      Token t = tokens[position];

    if (t.type != TKN_MULT && t.type != TKN_DIV) break;

    position++;
    *ast = (AST){
      .type = AST_TYPE_BINARY_OPERATOR,
      .astBO = {
        .operator = (t.type == TKN_MULT) ? OP_MULT : OP_DIV,
        .left = current,
        .right = parse_number(tokens),
      },
    };

      current = ast;
    }

  return current;
}

AST *parse_number(Token *tokens) {
  Token t = tokens[position++];
  
  AST *ast = malloc(sizeof(AST));

  *ast = (AST){
    .type = AST_TYPE_NUMBER,
    .astNum = {
      .number = t.num,
    },
  };

  return ast;
}


AST *run_parser(Token *tokens, int token_count) {
  parser_token_count = token_count;
  position = 0;

  parse_term2(tokens);

  return current;
}

// free all ASTs
void deinitAst(AST *ast) {
  if (ast->type == AST_TYPE_BINARY_OPERATOR) {
    deinitAst(ast->astBO.right);
    deinitAst(ast->astBO.left);
  }
  free(ast);
}
