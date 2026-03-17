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

enum UnaryOperatorType {
  OP_U_MINUS,
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
  enum UnaryOperatorType operator;
  Expression *operand;
} ExpUnaryOperator;

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

enum ExpressionType {
  EXPRESSION_BINARY_OPERATOR,
  EXPRESSION_UNARY_OPERATOR,
  EXPRESSION_NUMBER,
  EXPRESSION_VAR,
  EXPRESSION_CALL,
};

struct Expression {
  enum ExpressionType type;
  union {
    ExpBinaryOperator bo;
    ExpUnaryOperator uo;
    ExpNumber num;
    ExpVar var;
    ExpCall call;
  };
};

enum ParserErrorType {
  PARSER_ERR_UNEXPECTED_TOKEN,
  PARSER_ERR_UNEXPECTED_EOS,
};

typedef struct {
  enum ParserErrorType type;
  char *message;
  uint16_t position;
} ParserError;

ParserError parser_error_init(char *buf) {
  return  (ParserError){.message = buf};
}

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

    case EXPRESSION_UNARY_OPERATOR:
      assert(expr->uo.operator == OP_U_MINUS);

      *written += sprintf(out + *written, "%*sOP:-u\n", depth * 2, "");
      print_expr_recursive(expr->uo.operand, depth+1, out, written);

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

int parser_position;
int parser_token_count;

AST *run_parser(Token *tokens, int token_count, ParserError *err);
Statement *parse_statement(Token *tokens, ParserError *err);
Expression *parse_expression(Token *tokens, ParserError *err);
Expression *parse_term2(Token *tokens, ParserError *err);
Expression *parse_term1(Token *tokens, ParserError *err);
Expression *parse_term0(Token *tokens, ParserError *err);
Token *get_token(Token *tokens, ParserError *err);
Token *peek_token(Token *tokens, ParserError *err);

Token *expect_token(Token *tokens, enum TokenType type, ParserError *err);
char *expect_var(Token *token, ParserError *err);

#define AST_MUST(ast) \
    if (ast == NULL) { \
      return NULL; \
    }

// returns NULL on error
AST *run_parser(Token *tokens, int token_count, ParserError *err) {
  parser_token_count = token_count;
  parser_position = 0;
  size_t stmt_idx = 0;
  AST *ast = malloc(sizeof(AST) * MAX_AST_STMTS);

  while (parser_position < token_count) {
    Statement *stmt = parse_statement(tokens, err);

    AST_MUST(stmt);

    ast->stmts[stmt_idx++] = stmt;

    if (parser_position < token_count) {
      Token *t = expect_token(tokens, TKN_SEMICOLON, err);
      AST_MUST(t);
    }
  }

  ast->count = stmt_idx;

  return ast;
}


Statement *parse_statement(Token *tokens, ParserError *err) {
  Token *t = peek_token(tokens, err);
  AST_MUST(t);

  if (t->type == TKN_DEF) {
    Statement *stmt = malloc(sizeof(Statement));

    parser_position++;

    Definition *def = malloc(sizeof(Definition));

    char *name = expect_var(tokens, err);
    AST_MUST(name);

    AST_MUST(expect_token(tokens, TKN_LPAREN, err));

    char *arg = expect_var(tokens, err);
    AST_MUST(arg);

    AST_MUST(expect_token(tokens, TKN_RPAREN, err));
    AST_MUST(expect_token(tokens, TKN_ASSIGNMENT, err));

    Expression *body= parse_expression(tokens, err);
    AST_MUST(body);

    *def = (Definition){.name = name, .args = arg, .body = body};

    *stmt = (Statement){
      .type = STMT_DEF,
      .def = def,
    };

    return stmt;
  } else {
    Statement *stmt = malloc(sizeof(Statement));

    Expression *expr = parse_expression(tokens, err);
    AST_MUST(expr);

    *stmt = (Statement){.type = STMT_EXPR, .expr = expr};

    return stmt;
  }
}

Expression *parse_expression(Token *tokens, ParserError *err) {
  return parse_term2(tokens, err);
}

Expression *parse_term2(Token *tokens, ParserError *err) {
  Expression *current = parse_term1(tokens, err);
  AST_MUST(current);

  while (parser_position < parser_token_count) {
    Expression *expr = malloc(sizeof(Expression));

    Token *t = peek_token(tokens, err);
    AST_MUST(t);

    if (t->type != TKN_PLUS && t->type != TKN_MINUS) break;

    parser_position++;

    Expression *right = parse_term1(tokens, err);
    AST_MUST(right);

    *expr = (Expression){
      .type = EXPRESSION_BINARY_OPERATOR,
      .bo = {
        .operator = (t->type == TKN_PLUS) ? OP_PLUS : OP_MINUS,
        .left = current,
        .right = right,
      },
    };

    current = expr;
  }

  return current;
}

Expression *parse_term1(Token *tokens, ParserError *err) {
  Expression *current = parse_term0(tokens, err);
  AST_MUST(current);

  while (parser_position < parser_token_count) {
    Expression *expr = malloc(sizeof(Expression));

    Token *t = peek_token(tokens, err);
    AST_MUST(t);

    if (t->type != TKN_MULT && t->type != TKN_DIV) break;

    parser_position++;

    Expression *right = parse_term0(tokens, err);
    AST_MUST(right);

    *expr = (Expression){
      .type = EXPRESSION_BINARY_OPERATOR,
      .bo = {
        .operator = (t->type == TKN_MULT) ? OP_MULT : OP_DIV,
        .left = current,
        .right = right,
      },
    };

      current = expr;
    }

  return current;
}

Expression *parse_term0(Token *tokens, ParserError *err) {
  Token *t = get_token(tokens, err);
  AST_MUST(t);
  
  // Parse parenthesis
  if (t->type == TKN_LPAREN) {
    Expression *expr = parse_term2(tokens, err);
    AST_MUST(expr);

    AST_MUST(expect_token(tokens, TKN_RPAREN, err));

    return expr;
  }

  if (t->type == TKN_MINUS) {
    Expression *expr = malloc(sizeof(Expression));
    *expr = (Expression){
      .type = EXPRESSION_UNARY_OPERATOR,
      .uo = (ExpUnaryOperator){
          .operator = OP_U_MINUS,
          .operand = parse_term0(tokens, err),
        }
      };

      return expr;
  }

  if (t->type == TKN_VAR) {
    Token *t_next  = peek_token(tokens, err);

    if (t_next != NULL && t_next->type == TKN_LPAREN) {
      parser_position++;
      // Parse function call
      Expression *expr = malloc(sizeof(Expression));
      *expr = (Expression){
        .type = EXPRESSION_CALL,
        .call = {
          .name = t->var,
          .expr = parse_expression(tokens, err)
        },
      };

      AST_MUST(expect_token(tokens, TKN_RPAREN, err));

      return expr;
    } else {
      // Parse variable
      Expression *expr = malloc(sizeof(Expression));

      *expr = (Expression){
        .type = EXPRESSION_VAR,
        .var = {
          .name = t->var,
        },
      };

      return expr;
    }
  }


  if (t->type == TKN_NUMBER) {
    Expression *expr = malloc(sizeof(Expression));

    *expr = (Expression){
      .type = EXPRESSION_NUMBER,
      .num = {
        .number = t->num,
      },
    };

    return expr;
  }

  err->type = PARSER_ERR_UNEXPECTED_TOKEN;
  err->position = t->position;
  sprintf(err->message, "Unexpected token: %s\n", token_type_name[t->type]);
  return NULL;
}

Token *expect_token(Token *tokens, enum TokenType type, ParserError *err) {
    Token t = tokens[parser_position++];

    if (t.type != type) {
      err->type = PARSER_ERR_UNEXPECTED_TOKEN;
      err->position = t.position;
       sprintf(err->message, "Expected type: %s, got %s\n", token_type_name[type], token_type_name[t.type]);
       return NULL;
    }

    return tokens+parser_position;
}

char *expect_var(Token *tokens, ParserError *err) {
    Token t = tokens[parser_position++];

    if (t.type != TKN_VAR) {
      err->type = PARSER_ERR_UNEXPECTED_TOKEN;
      err->position = t.position;
       sprintf(err->message, "Expected variable, got %s\n", token_type_name[t.type]);
       return NULL;
    }

    return t.var;
}

Token *get_token(Token *tokens, ParserError *err) {
  Token *t = peek_token(tokens, err);
  AST_MUST(t);

  parser_position++;

  return t;
}

Token *peek_token(Token *tokens, ParserError *err) {
  if (parser_position >= parser_token_count) {
    err->type = PARSER_ERR_UNEXPECTED_EOS;
    sprintf(err->message, "Unexpected EOS\n");

    const Token last_token = tokens[parser_token_count-1];
    uint16_t position = last_token.position;
    err->position = position;

    return NULL;
  }

  return tokens+parser_position;
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
