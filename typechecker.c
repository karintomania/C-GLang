#include <stdint.h>
#ifndef UNITY_BUILD
  #include "parser.c"
  #include "stb_ds.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#define MAX_ARG_LEN 32

typedef struct Type Type;

enum TypeName {
  TYPE_NUM,
  TYPE_FUNC,
  TYPE_UNKNOWN,
};

struct Type {
  enum TypeName type;
  // These fields are only for function
  // not using union for convenience
  Type **args;
  size_t arg_len;
  Type *result;
};

typedef struct {
  char *key;
  Type *value;
} TypeMap;


enum TypeErrorType {
  TYPE_ERR_TYPE_MISMATCH,
  TYPE_ERR_UNDEFINED_VAR,
  TYPE_ERR_UNDEFINED_FUNC,
  TYPE_ERR_EXPR_NOT_ALLOWED,
};

typedef struct {
  Type *want;
  Type *got;
} TypeErrorTypeMismatch;

typedef struct {
  char *var;
} TypeErrorUndefinedVariable;

typedef struct {
  char *func;
} TypeErrorUndefinedFunction;


typedef struct {
  enum TypeErrorType err_type;
  size_t position;
  char *message;
  union {
    TypeErrorTypeMismatch mm;
    TypeErrorUndefinedVariable uv;
    TypeErrorUndefinedFunction uf;
    // ExpressionNotAllowed doesn't have extra info, so not in the union 
  };
} TypeError;

TypeError type_error_init(char *buf) {
  return  (TypeError){.message = buf};
}

#define TYPE_MUST(type) \
    if (type == NULL) { \
      return NULL; \
    }


Type *unify(Type *want, Type *got, TypeError *err);
Type *typecheck_expression( Expression *expr, Type *want, TypeMap *dtm, TypeMap *atm, TypeError *err);
Type *run_typecheck(AST *ast, TypeError *err);

Type type_unknown = (Type){.type = TYPE_UNKNOWN};
Type type_number = (Type){.type = TYPE_NUM};

// return NULL on error
Type *unify(Type *want, Type *got, TypeError *err) {
  if (want->type == TYPE_UNKNOWN) return got;
  if (got->type == TYPE_UNKNOWN) return want;

  if (want->type == TYPE_NUM && got->type == TYPE_NUM) {
    return &type_number;
  }

  if (want->type == TYPE_FUNC && got->type == TYPE_FUNC) {
    if (want->arg_len != got->arg_len) {
      *err = (TypeError) {
        .mm = (TypeErrorTypeMismatch) {.got = got, .want = want}
      };
      return NULL;
    }

    Type **unified_args = malloc(sizeof(Type *) * want->arg_len);

    for (size_t i = 0; i < want->arg_len; i ++) {
      Type *arg = unify(want->args[i], got->args[i], err);
      if (arg == NULL) return NULL;
      unified_args[i] = arg;
    }

    Type *unified_result = unify(want->result, got->result, err);
    if (unified_result == NULL) return NULL;

    Type *t = malloc(sizeof(Type));
    *t = (Type){
      .type = TYPE_FUNC,
      .args = unified_args,
      .arg_len = want->arg_len,
      .result = unified_result,
    };
    return t;
  }

  *err = (TypeError) {
    .err_type = TYPE_ERR_TYPE_MISMATCH,
    .mm = (TypeErrorTypeMismatch) {.got = got, .want = want}
  };

  return NULL;
}

Type *typecheck_expression(
  Expression *expr,
  Type *want,
  TypeMap *def_map,
  TypeMap *assignment_map,
  TypeError *err
) {
  if (expr->type == EXPRESSION_NUMBER) {
    return &type_number;
  }

  if (expr->type == EXPRESSION_BINARY_OPERATOR) {
    // TODO: null check
    typecheck_expression(expr->bo.left, &type_number, def_map, assignment_map, err);
    typecheck_expression(expr->bo.right, &type_number, def_map, assignment_map, err);

    return &type_number;
  }

  if (expr->type == EXPRESSION_UNARY_OPERATOR) {
    Type *operand  = malloc(sizeof(Type));

    // TODO: null check
    operand = typecheck_expression(expr->uo.operand, &type_number, def_map, assignment_map, err);

    return operand;
  }

  if (expr->type == EXPRESSION_VAR) {
    if (shgeti(def_map, expr->var.name) != -1) {
      return shget(def_map, expr->var.name);
    }
    
    if (shgeti(assignment_map, expr->var.name) != -1) {
      return shget(assignment_map, expr->var.name);
    }
    
    *err = (TypeError){
      .err_type = TYPE_ERR_UNDEFINED_VAR,
      .uv = (TypeErrorUndefinedVariable){.var = expr->var.name}
    };
    return NULL;
  }

  if (expr->type == EXPRESSION_CALL) {
    // TODO: multi args
    Type *arg = typecheck_expression(expr->call.args, &(Type){.type = TYPE_NUM}, def_map, assignment_map, err);

    if(shgeti(def_map, expr->call.name) == -1) {
      *err = (TypeError){
        .err_type = TYPE_ERR_UNDEFINED_FUNC,
        .uf = (TypeErrorUndefinedFunction){.func = expr->call.name}
      };
      return NULL;
    }

    Type *def = shget(def_map, expr->call.name);

    Type *unified_def = unify(
      &(Type){.type = TYPE_FUNC, .args = &arg, .arg_len = 1, .result = want},
      def,
      err
    );

    return unified_def;
  }

  fprintf(stderr, "The code shouldn't reach here\n");
  exit(1);
}

Type *run_typecheck(AST *ast, TypeError *err) {
  TypeMap *def_map = NULL;

  for (size_t i = 0; i < ast->count; i++) {
    Statement *stmt = ast->stmts[i];

    if (stmt->type == STMT_DEF) {
      Definition *def = stmt->def;
      TypeMap *assignment_map = NULL;

      // TODO: multi args
      Type *want_args[1];

      shput(assignment_map, def->args, &type_number);

      want_args[0] = &type_number;

      Type *result = typecheck_expression(
        def->body,
        &type_number,
        def_map,
        assignment_map,
        err
      );

      Type *func_type =  &(Type){
        .type = TYPE_FUNC,
        .args = want_args,
        .arg_len = 1,
        .result = result,
      };

      TYPE_MUST(func_type);

      shput(
        def_map,
        def->name,
        func_type
      );
    }

    if (stmt->type == STMT_EXPR) {
      if (i != ast->count-1) {
        *err = (TypeError){.err_type = TYPE_ERR_EXPR_NOT_ALLOWED};
        return NULL;
      }

      Type *res = typecheck_expression(stmt->expr, &(Type){.type = TYPE_UNKNOWN}, def_map, NULL, err);

      return res;
    }
  }

  fprintf(stderr, "The code shouldn't reach here\n");
  exit(1);
}
