#ifndef UNITY_BUILD
#include "lexer.c"
#endif

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

enum OperatorType {
  OP_PLUS,
  OP_MULT,
  OP_DIV,
  OP_MINUS,
};

typedef struct Expression Expression;
typedef struct AST AST;

typedef struct {
  enum OperatorType operator;
  Expression *left;
  Expression *right;
} ExpBinaryOperator;

typedef struct {
  float number;
} ExpNumber;

typedef struct {
  char *name;
} ExpVar;

typedef struct {
  Expression *expressions;
} ExpCall;

enum ExpressionType {
  EXPRESSION_TYPE_BINARY_OPERATOR,
  EXPRESSION_TYPE_NUMBER,
  EXPRESSION_TYPE_VAR,
  EXPRESSION_TYPE_CALL,
};

struct Expression {
  enum ExpressionType type;
  union {
    ExpBinaryOperator expBO;
    ExpNumber expNum;
    ExpVar expVar;
    ExpCall expCall;
  };
};

void print_ast_recursive(Expression *ast, int depth, char *out, uint16_t *written) {
  switch (ast->type) {
    char *op_str;
    case EXPRESSION_TYPE_BINARY_OPERATOR:
      switch (ast->expBO.operator) {
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

      *written += sprintf(out + *written, "%*sOP:%s\n", depth * 2, "", op_str);
      print_ast_recursive(ast->expBO.left, depth+1, out, written);
      print_ast_recursive(ast->expBO.right, depth+1, out, written);

      break;
    case EXPRESSION_TYPE_NUMBER:
      *written += sprintf(out + *written, "%*sNUM:%g\n", depth * 2, "", ast->expNum.number);
      break;
    case EXPRESSION_TYPE_VAR:
      *written += sprintf(out + *written, "%*sVAR:%s\n", depth * 2, "", ast->expVar.name);
      break;
    case EXPRESSION_TYPE_CALL:
      // TODO:update here. no op for now
      break;
  }
}

uint16_t sprint_ast(Expression *ast, char *out) {
  uint16_t written = 0;
  print_ast_recursive(ast, 0, out, &written);

  return written;
}

void print_ast(Expression *ast) {
  char buf[4000];
  sprint_ast(ast, buf);

  printf("%s", buf);
}

/*---------------------
 Main Parse logic
------------------------*/

int position;
Expression *current;
int parser_token_count;

Expression *parse_expression(Token *tokens);
Expression *parse_term2(Token *tokens);
Expression *parse_term1(Token *tokens);
Expression *parse_term0(Token *tokens);

Expression *parse_expression(Token *tokens) {
  return parse_term2(tokens);
}

Expression *parse_term2(Token *tokens) {
  current = parse_term1(tokens);

  while (position < parser_token_count) {
    Expression *ast = malloc(sizeof(Expression));

    Token t = tokens[position];

    if (t.type != TKN_PLUS && t.type != TKN_MINUS) break;

    position++;
    *ast = (Expression){
      .type = EXPRESSION_TYPE_BINARY_OPERATOR,
      .expBO = {
        .operator = (t.type == TKN_PLUS) ? OP_PLUS : OP_MINUS,
        .left = current,
        .right = parse_term1(tokens),
      },
    };

    current = ast;
  }

  return current;
}

Expression *parse_term1(Token *tokens) {
  current = parse_term0(tokens);

  while (position < parser_token_count) {
      Expression *ast = malloc(sizeof(Expression));

      Token t = tokens[position];

    if (t.type != TKN_MULT && t.type != TKN_DIV) break;

    position++;
    *ast = (Expression){
      .type = EXPRESSION_TYPE_BINARY_OPERATOR,
      .expBO = {
        .operator = (t.type == TKN_MULT) ? OP_MULT : OP_DIV,
        .left = current,
        .right = parse_term0(tokens),
      },
    };

      current = ast;
    }

  return current;
}

Expression *parse_term0(Token *tokens) {
  Token t = tokens[position++];
  
  if (t.type == TKN_LPAREN) {
    current = parse_term2(tokens);

    t = tokens[position++];
    assert(t.type == TKN_RPAREN);

    return current;
  }

  if (t.type == TKN_VAR) {
    Expression *ast = malloc(sizeof(Expression));
    *ast = (Expression){
      .type = EXPRESSION_TYPE_VAR,
      .expVar = {
        .name = t.var,
      },
    };

    return ast;
  }


  bool is_negative = false;

  if (t.type == TKN_MINUS) {
    is_negative = true;
    t = tokens[position++];
  }

  if (t.type == TKN_NUMBER) {
    Expression *ast = malloc(sizeof(Expression));

    *ast = (Expression){
      .type = EXPRESSION_TYPE_NUMBER,
      .expNum = {
        .number = t.num * (is_negative ? -1 : 1),
      },
    };
    return ast;
  }

  return NULL; // shouldn't reach here.
}


Expression *run_parser(Token *tokens, int token_count) {
  parser_token_count = token_count;
  position = 0;

  parse_term2(tokens);

  return current;
}

// free all Expressions
void deinit_ast(Expression *ast) {
  if (ast->type == EXPRESSION_TYPE_BINARY_OPERATOR) {
    deinit_ast(ast->expBO.right);
    deinit_ast(ast->expBO.left);
  }
  free(ast);
}
