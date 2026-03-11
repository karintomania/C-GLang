#ifndef UNITY_BUILD
#include "lexer.c"
#endif

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// max count of statements in AST
#define MAX_AST_STMTS 100

enum StatementType {
  STMT_EXPR,
  STMT_DEF,
};

const char* statement_type_name[] = {
  "STMT_EXPR",
  "STMT_DEF",
};

enum OperatorType {
  OP_PLUS,
  OP_MULT,
  OP_DIV,
  OP_MINUS,
};

const char* operator_type_name[] =  {
  "OP_PLUS",
  "OP_MULT",
  "OP_DIV",
  "OP_MINUS",
};

typedef struct Statement Statement;
typedef struct Expression Expression;
typedef struct Definition Definition;
typedef struct AST AST;

struct Statement {
  enum StatementType type;
  union {
    Expression *expr;
    Definition *def;
  };
};

struct AST {
  Statement *stmts[MAX_AST_STMTS];
  size_t count;
};


struct Definition {
  char *name;
  char *args; // TODO support multiple args later
  Expression *body;
};

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
  char *name;
  Expression *expr;
} ExpCall;

// TODO: Shorten names to EXPR_VAR
enum ExpressionType {
  EXPRESSION_BINARY_OPERATOR,
  EXPRESSION_NUMBER,
  EXPRESSION_VAR,
  EXPRESSION_CALL,
};

struct Expression {
  enum ExpressionType type;
  union {
    ExpBinaryOperator bo;
    ExpNumber num;
    ExpVar var;
    ExpCall call;
  };
};

void print_stmt(Statement *stmt, int depth, char *out, uint16_t *written);
void print_expr_recursive(Expression *expr, int depth, char *out, uint16_t *written);

void print_stmt(Statement *stmt, int depth, char *out, uint16_t *written) {
  if (stmt->type == STMT_DEF) {
    Definition *def = stmt->def;
    *written += sprintf(out + *written, "%*sDEF:%s, ARGS:%s\n", depth * 2, "", def->name, def->args);
    *written += sprintf(out + *written, "%*sBODY:\n", ++depth * 2, "");
      print_expr_recursive(def->body, depth+1, out, written);
  } else {
    Expression *expr = stmt->expr;
    print_expr_recursive(expr, depth, out, written);
  }
}

void print_expr_recursive(Expression *expr, int depth, char *out, uint16_t *written) {
  switch (expr->type) {
    char *op_str;
    case EXPRESSION_BINARY_OPERATOR:
      switch (expr->bo.operator) {
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
      print_expr_recursive(expr->bo.left, depth+1, out, written);
      print_expr_recursive(expr->bo.right, depth+1, out, written);

      break;
    case EXPRESSION_NUMBER:
      *written += sprintf(out + *written, "%*sNUM:%g\n", depth * 2, "", expr->num.number);
      break;
    case EXPRESSION_VAR:
      *written += sprintf(out + *written, "%*sVAR:%s\n", depth * 2, "", expr->var.name);
      break;
    case EXPRESSION_CALL:
      // TODO:update here. no op for now
      *written += sprintf(out + *written, "%*sCALL:%s\n", depth * 2, "", expr->call.name);
      print_expr_recursive(expr->call.expr, depth+1, out, written);
      break;
  }
}

uint16_t sprint_ast(AST *ast, char *out) {
  uint16_t written = 0;
  if (ast == NULL) {
    written = sprintf(out, "ast is null.");
    return written;
  }

  for (size_t i = 0; i < ast->count; i++) {
    Statement *stmt = ast->stmts[i];
    print_stmt(stmt, 0, out, &written);
  }


  return written;
}

void print_ast(AST *ast) {
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

AST *run_parser(Token *tokens, int token_count);
Statement *parse_statement(Token *tokens);
Expression *parse_expression(Token *tokens);
Expression *parse_term2(Token *tokens);
Expression *parse_term1(Token *tokens);
Expression *parse_term0(Token *tokens);

Token expect_token(Token *tokens, enum TokenType type);
char *expect_var(Token *token);

AST *run_parser(Token *tokens, int token_count) {
  parser_token_count = token_count;
  position = 0;
  size_t stmt_idx = 0;
  AST *ast = malloc(sizeof(AST) * MAX_AST_STMTS);

  while (position < token_count) {
    Statement *stmt = parse_statement(tokens);

    ast->stmts[stmt_idx++] = stmt;

    if (position < token_count) {
      expect_token(tokens, TKN_SEMICOLON);
    }
  }

  ast->count = stmt_idx;

  return ast;
}


Statement *parse_statement(Token *tokens) {
  Token t = tokens[position];

  if (t.type == TKN_DEF) {
    Statement *stmt = malloc(sizeof(Statement));

    position++;

    Definition *def =malloc(sizeof(Definition));

    char *name = expect_var(tokens);

    expect_token(tokens, TKN_LPAREN);

    char *arg = expect_var(tokens);

    expect_token(tokens, TKN_RPAREN);
    expect_token(tokens, TKN_ASSIGNMENT);

    Expression *body= parse_expression(tokens);

    *def = (Definition){.name = name, .args = arg, .body = body};

    *stmt = (Statement){
      .type = STMT_DEF,
      .def = def,
    };

    return stmt;
  } else {
    Statement *stmt = malloc(sizeof(Statement));

    Expression *expr = parse_expression(tokens);

    *stmt = (Statement){.type = STMT_EXPR, .expr = expr};

    return stmt;
  }
}

Expression *parse_expression(Token *tokens) {
  return parse_term2(tokens);
}

Expression *parse_term2(Token *tokens) {
  current = parse_term1(tokens);

  while (position < parser_token_count) {
    Expression *expr = malloc(sizeof(Expression));

    Token t = tokens[position];

    if (t.type != TKN_PLUS && t.type != TKN_MINUS) break;

    position++;
    *expr = (Expression){
      .type = EXPRESSION_BINARY_OPERATOR,
      .bo = {
        .operator = (t.type == TKN_PLUS) ? OP_PLUS : OP_MINUS,
        .left = current,
        .right = parse_term1(tokens),
      },
    };

    current = expr;
  }

  return current;
}

Expression *parse_term1(Token *tokens) {
  current = parse_term0(tokens);

  while (position < parser_token_count) {
      Expression *expr = malloc(sizeof(Expression));

      Token t = tokens[position];

    if (t.type != TKN_MULT && t.type != TKN_DIV) break;

    position++;
    *expr = (Expression){
      .type = EXPRESSION_BINARY_OPERATOR,
      .bo = {
        .operator = (t.type == TKN_MULT) ? OP_MULT : OP_DIV,
        .left = current,
        .right = parse_term0(tokens),
      },
    };

      current = expr;
    }

  return current;
}

Expression *parse_term0(Token *tokens) {
  Token t = tokens[position++];
  
  // Parse parenthesis
  if (t.type == TKN_LPAREN) {
    current = parse_term2(tokens);

    expect_token(tokens, TKN_RPAREN);

    return current;
  }

  if (t.type == TKN_VAR) {
    Token t_next  = tokens[position];

    if (t_next.type == TKN_LPAREN) {
      position++;
      // Parse function call
      Expression *expr = malloc(sizeof(Expression));
      *expr = (Expression){
        .type = EXPRESSION_CALL,
        .call = {
          .name = t.var,
          .expr = parse_expression(tokens)
        },
      };

      expect_token(tokens, TKN_RPAREN);

      return expr;
    } else {
      // Parse variable
      Expression *expr = malloc(sizeof(Expression));

      *expr = (Expression){
        .type = EXPRESSION_VAR,
        .var = {
          .name = t.var,
        },
      };

      return expr;
    }
  }

  // parse number
  bool is_negative = false;

  if (t.type == TKN_MINUS) {
    is_negative = true;
    t = tokens[position++];
  }

  if (t.type == TKN_NUMBER) {
    Expression *expr = malloc(sizeof(Expression));

    *expr = (Expression){
      .type = EXPRESSION_NUMBER,
      .num = {
        .number = t.num * (is_negative ? -1 : 1),
      },
    };
    return expr;
  }

  return NULL; // shouldn't reach here.
}

Token expect_token(Token *tokens, enum TokenType type) {
    Token t = tokens[position++];

    if (t.type != type) {
       fprintf(stderr, "Expected type: %s, got %s\n", token_type_name[type], token_type_name[t.type]);
       assert(t.type == type);
    }

    return t;
}

char *expect_var(Token *tokens) {
    Token t = tokens[position++];

    if (t.type != TKN_VAR) {
       fprintf(stderr, "Expected variable, got %s\n", token_type_name[t.type]);
       assert(t.type == TKN_VAR);
    }

    return t.var;
}

void deinit_statement(Statement *stmt);
void deinit_expression(Expression *expr);
void deinit_definition(Definition *def);

// free all Expressions
void deinit_ast(AST *ast) {
  // TODO: multiple statements
  Statement *stmt = ast->stmts[0];
  deinit_statement(stmt);
  free(ast);
}

void deinit_statement(Statement *stmt) {
  if (stmt->type == STMT_EXPR) {
    deinit_expression(stmt->expr);
  }

  if (stmt->type == STMT_DEF) {
    deinit_expression(stmt->def->body);
    free(stmt->def->name);
    free(stmt->def->args);
    free(stmt->def);
  }
  free(stmt);
}

void deinit_expression(Expression *expr){
  if (expr->type == EXPRESSION_CALL) {
    deinit_expression(expr->call.expr);
    free(expr->call.name);
  }
  if (expr->type == EXPRESSION_VAR) {
    free(expr->var.name);
  }
  if (expr->type == EXPRESSION_BINARY_OPERATOR) {
    deinit_expression(expr->bo.right);
    deinit_expression(expr->bo.left);
  }
  free(expr);
}
