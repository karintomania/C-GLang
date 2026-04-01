#include <stdint.h>
#ifndef UNITY_BUILD
  #include "parser.c"
  #include "typechecker.c"
  #include "stb_ds.h"
#endif

typedef struct Number Number;
typedef struct Function Function;

enum ValueType {
  VAL_NUM,
  VAL_FUNC
};

struct Function {
  char *name;
  char **args;
  uint16_t args_len;
  Expression *body;
};

typedef struct {
  enum ValueType type;
  union {
    float num;
    Function func;
  };
} Value;

typedef struct {
  char *key;
  Definition *value;
} DefMap;

typedef struct {
  Value v;
  BuiltinMap *bm;
  DefMap *dm;
} InterpretResult;

typedef struct {
  char *key;
  float value;
} Assignment;

InterpretResult  interpret(AST *ast);
Value interpretExpression(Expression *ast, DefMap *dm, Assignment *assignment, BuiltinMap *bm);
float expectNumber(Value v);

InterpretResult interpret(AST *ast) {
  DefMap *dm = NULL;
  BuiltinMap *bm = NULL;

  BuiltinSlice builtins = get_builtins();

  for (uint16_t i = 0; i < builtins.count; i++) {
    Builtin *b = builtins.builtins + i;
    shput(bm, b->impl_name, b);
    shput(dm, b->name, b->def);
  }

  for (size_t i = 0; i < ast->count; i++) {
    Statement *stmt = ast->stmts[i];
    if (stmt->type == STMT_DEF) {
      Definition *def = stmt->def;
      shput(dm, def->name, def);
    }
    if (stmt->type == STMT_EXPR) {
      Value v = interpretExpression(stmt->expr, dm, NULL, bm);
      return (InterpretResult){.v = v, .bm = bm, .dm = dm};
    }
  }

  fprintf(stderr, "Unreachable\n");
  assert(0);
}

Value interpretExpression(Expression *expr, DefMap *dm, Assignment *assignment, BuiltinMap *bm) {

  if (expr->type == EXPRESSION_UNARY_OPERATOR) {
    float operand = expectNumber(interpretExpression(expr->uo.operand, dm, assignment, bm));

    return (Value){.type = VAL_NUM, .num =  operand * -1};
  }

  if (expr->type == EXPRESSION_CALL) {
    float value = expectNumber(interpretExpression(expr->call.args, dm, assignment, bm));

    if (shgeti(dm, expr->call.name) != -1) {
      Definition *d = shget(dm, expr->call.name);

      // TODO: multi args
      shput(assignment, d->args[0], value);

      return interpretExpression(d->body, dm, assignment, bm);
    } else if (shgeti(bm, expr->var.name) != -1) {
      Builtin *b = shget(bm, expr->var.name);
      float res = b->body(value);

      return (Value){.type = VAL_NUM, .num = res};
    } else  {
      assert(0);
    }
  }

  if (expr->type == EXPRESSION_VAR) {
    if (shgeti(assignment, expr->var.name) != -1) {
      return (Value){.type = VAL_NUM, .num = shget(assignment, expr->var.name)};
    } else if (shgeti(dm, expr->var.name) != -1) {
      Definition *def  = shget(dm, expr->var.name);

      return (Value){
        .type = VAL_FUNC,
        .func = (Function){
          .args = def->args,
          .args_len = def->args_len,
          .name = expr->var.name,
          .body = def->body,
        },
      };
    } else {
      assert(0);
    }
  }

  if (expr->type == EXPRESSION_NUMBER) {
    return (Value){.type = VAL_NUM, .num = expr->num.number};
  }

  if (expr->type == EXPRESSION_BINARY_OPERATOR) {
    float value;
    switch (expr->bo.operator) {
      case OP_PLUS:
        value = expectNumber(interpretExpression(expr->bo.left, dm, assignment, bm))
               + expectNumber(interpretExpression(expr->bo.right, dm, assignment, bm));
        break;
      case OP_MINUS:
        value = expectNumber(interpretExpression(expr->bo.left, dm, assignment, bm))
               - expectNumber(interpretExpression(expr->bo.right, dm, assignment, bm));
        break;
      case OP_MULT:
        value = expectNumber(interpretExpression(expr->bo.left, dm, assignment, bm))
               * expectNumber(interpretExpression(expr->bo.right, dm, assignment, bm));
        break;
      case OP_DIV:
        value = expectNumber(interpretExpression(expr->bo.left, dm, assignment, bm))
               / expectNumber(interpretExpression(expr->bo.right, dm, assignment, bm));
        break;
    }
    return (Value){.type = VAL_NUM, .num =  value};
  }

  printf("unknown ast.");
  assert(0);
}

float expectNumber(Value v) {
  assert(v.type == VAL_NUM);

  return v.num;
}
