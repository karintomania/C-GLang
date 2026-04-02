#include <math.h>
#ifndef UNITY_BUILD
  #include "lexer.c"
  #include "parser.c"
  #include "stb_ds.h"
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ARG_LEN 32

typedef struct Type Type;

typedef struct Builtin Builtin;

typedef struct BuiltinSlice BuiltinSlice;

enum TypeName {
  TYPE_NUM,
  TYPE_FUNC,
  TYPE_UNKNOWN,
};

const char* type_name_str[] = {
  "TYPE_NUM",
  "TYPE_FUNC",
  "TYPE_UNKNOWN",
};

struct Type {
  enum TypeName type;
  // These fields are only for function
  // not using union for convenience
  Type **args;
  size_t args_len;
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
  uint16_t position;
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

struct Builtin {
  char *name;
  // func name prefixed with _builtin_
  char *impl_name;
  Type *type;
  Definition *def;
  uint16_t args_len;
  float (*body)(int count, float *args);
};

typedef struct {
  char *key;
  Builtin *value;
} BuiltinMap;

struct BuiltinSlice{
  Builtin *builtins;
  uint16_t count;
};

struct BuiltinTypeSlice{
  Type *types;
  uint16_t count;
};

#define TYPE_MUST(type) \
    if (type == NULL) { \
      return NULL; \
    }


Type *unify(Type *want, Type *got, uint16_t position, TypeError *err);
Type *typecheck_expression( Expression *expr, Type *want, TypeMap *dtm, TypeMap *atm, TypeError *err);
Type *run_typechecker(AST *ast, TypeError *err);

BuiltinSlice get_builtins(void);

Type type_unknown = (Type){.type = TYPE_UNKNOWN};
Type type_number = (Type){.type = TYPE_NUM};

// return NULL on error
Type *unify(Type *want, Type *got, uint16_t position, TypeError *err) {
  if (want->type == TYPE_UNKNOWN) return got;
  if (got->type == TYPE_UNKNOWN) return want;

  if (want->type == TYPE_NUM && got->type == TYPE_NUM) {
    return &type_number;
  }

  if (want->type == TYPE_FUNC && got->type == TYPE_FUNC) {
    if (want->args_len != got->args_len) {
      err->err_type = TYPE_ERR_TYPE_MISMATCH;
      err->position = position;
      err->mm = (TypeErrorTypeMismatch) {.got = got, .want = want};
      sprintf(err->message, "Type mismatch. want: %s with %zd args, got %s with %zd args", type_name_str[want->type], want->args_len, type_name_str[got->type], got->args_len);
      return NULL;
    }

    Type **unified_args = malloc(sizeof(Type *) * want->args_len);

    for (size_t i = 0; i < want->args_len; i ++) {
      Type *arg = unify(want->args[i], got->args[i], position, err);
      TYPE_MUST(arg);
      unified_args[i] = arg;
    }

    Type *unified_result = unify(want->result, got->result, position, err);
    TYPE_MUST(unified_result);

    Type *t = malloc(sizeof(Type));
    *t = (Type){
      .type = TYPE_FUNC,
      .args = unified_args,
      .args_len = want->args_len,
      .result = unified_result,
    };
    return t;
  }

  err->err_type = TYPE_ERR_TYPE_MISMATCH;
      err->position = position;
  err->mm = (TypeErrorTypeMismatch) {.got = got, .want = want};
  sprintf(err->message, "Type mismatch. want: %s, got %s", type_name_str[want->type], type_name_str[got->type]);

  return NULL;
}

Type *typecheck_expression(
  Expression *expr,
  Type *want,
  TypeMap *def_map,
  TypeMap *assignment_map,
  TypeError *err
) {
  uint16_t position = expr->position;

  if (expr->type == EXPRESSION_NUMBER) {
    return &type_number;
  }

  if (expr->type == EXPRESSION_BINARY_OPERATOR) {
    // TODO: null check
    TYPE_MUST(typecheck_expression(expr->bo.left, &type_number, def_map, assignment_map, err));
    TYPE_MUST(typecheck_expression(expr->bo.right, &type_number, def_map, assignment_map, err));

    return &type_number;
  }

  if (expr->type == EXPRESSION_UNARY_OPERATOR) {
    Type *operand  = malloc(sizeof(Type));

    // TODO: null check
    operand = typecheck_expression(expr->uo.operand, &type_number, def_map, assignment_map, err);

    TYPE_MUST(operand);

    return operand;
  }

  if (expr->type == EXPRESSION_VAR) {
    if (shgeti(def_map, expr->var.name) != -1) {
      Type *got = shget(def_map, expr->var.name);

      return unify(want, got, position, err);
    }
    
    if (shgeti(assignment_map, expr->var.name) != -1) {
      Type *got = shget(assignment_map, expr->var.name);

      return unify(want, got, position, err);
    }
    
    err->err_type = TYPE_ERR_UNDEFINED_VAR;
    err->position = position;
    err->uv = (TypeErrorUndefinedVariable){.var = expr->var.name};
    sprintf(err->message, "Undefined variable: %s", expr->var.name);
    return NULL;
  }

  if (expr->type == EXPRESSION_CALL) {
    if(shgeti(def_map, expr->call.name) == -1) {
      err->err_type = TYPE_ERR_UNDEFINED_FUNC;
      err->position = position;
      err->uf = (TypeErrorUndefinedFunction){.func = expr->call.name};
      sprintf(err->message, "Undefined function: %s", expr->call.name);
      return NULL;
    }

    Type *def = shget(def_map, expr->call.name);

    Type **args = malloc(sizeof(Type *) * MAX_ARG_LEN);

    for (uint16_t i = 0; i < expr->call.args_len; i++) {
      Type *arg = typecheck_expression(expr->call.args[i], &(Type){.type = TYPE_NUM}, def_map, assignment_map, err);
      TYPE_MUST(arg);
      args[i] = arg;
    }

    Type *unified_def = unify(
      &(Type){
        .type = TYPE_FUNC,
        .args = args,
        .args_len = expr->call.args_len,
        .result = want
      },
      def,
      position,
      err
    );

    TYPE_MUST(unified_def);

    return unify(want, unified_def->result, position, err);
  }

  fprintf(stderr, "Unreachable\n");
  assert(0);
}

Type *run_typechecker(AST *ast, TypeError *err) {
  TypeMap *def_map = NULL;

  BuiltinSlice builtins = get_builtins();
  for (size_t i = 0; i < builtins.count; i++) {
    Builtin builtin = builtins.builtins[i];
    shput(def_map, builtin.name, builtin.type);
  }

  for (size_t i = 0; i < ast->count; i++) {
    Statement *stmt = ast->stmts[i];

    if (stmt->type == STMT_DEF) {
      Definition *def = stmt->def;
      TypeMap *assignment_map = NULL;

      Type **want_args = malloc(sizeof(Type *) * def->args_len);
      assert(want_args != NULL);

      for (uint16_t i = 0; i < def->args_len; i++) {
        shput(assignment_map, def->args[i], &type_number);
        want_args[i] = &type_number;
      }

      // register temporary definition for recursive function
      Type *tmp_def = &(Type){
        .type = TYPE_FUNC,
        .args = want_args,
        .args_len = def->args_len,
        .result = &type_unknown,
      };

      shput(def_map, def->name, tmp_def);

      Type *result = typecheck_expression(
        def->body,
        &type_number,
        def_map,
        assignment_map,
        err
      );

      TYPE_MUST(result);

      Type *func_type = malloc(sizeof(Type));
      assert(func_type != NULL);

      *func_type = (Type){
        .type = TYPE_FUNC,
        .args = want_args,
        .args_len = def->args_len,
        .result = result,
      };

      shput(
        def_map,
        def->name,
        func_type
      );
    }

    if (stmt->type == STMT_EXPR) {
      if (i != ast->count-1) {
        err->err_type = TYPE_ERR_EXPR_NOT_ALLOWED;
        err->position = stmt->expr->position;
        sprintf(err->message, "Expression not allowed other than the end of the program.");
        return NULL;
      }

      Type *res = typecheck_expression(stmt->expr, &(Type){.type = TYPE_UNKNOWN}, def_map, NULL, err);

      return res;
    }
  }

  fprintf(stderr, "Unreachable\n");
  assert(0);
}


/*---------------------
 Builtin Implementation
------------------------*/

Builtin builtin_storage[128];

Builtin init_builtin(
  const char *name,
  uint16_t args_len,
  float (*body)(int count, float *args),
  const char *def
) {
  char *owned_name = strndup(name, strlen(name)+1);
  assert(owned_name != NULL);

  uint16_t len = strlen(name) + 10;
  char *impl_name = malloc(sizeof(char) * len);
  assert(impl_name != NULL);
  sprintf(impl_name, "_builtin_%s", name);

  Type *type = malloc(sizeof(Type));
  Type **args = malloc(sizeof(Type *) * args_len);

  for (uint16_t i=0; i < args_len; i++) {
    // currenlty all args are number type
    args[i] = &type_number;
  }

  *type = (Type){
    .type = TYPE_FUNC,
    .args = args,
    .args_len = args_len,
    .result = &type_number
  };

  Token tokens[128] = {0};
  char err_buf[256] = {0};
  LexerError lexer_err = lexer_error_init(err_buf);
  ParserError parser_err = parser_error_init(err_buf);

  int16_t token_count = run_lexer(def, tokens, &lexer_err);
  assert(token_count != LEXER_ERROR);

  AST *ast = run_parser(tokens, token_count, &parser_err);
  assert(ast != NULL);

  Definition *d = malloc(sizeof(Definition));

  *d = *(ast->stmts[0]->def);

  return (Builtin){
    .name = owned_name,
    .impl_name = impl_name,
    .def = d,
    .type = type,
    .args_len = args_len,
    .body = body,
  };
}

float sin_impl(int count, float *args) {
  assert(count == 1);
  float f = args[0];
  return sinf(f);
}

float cos_impl(int count, float *args) {
  assert(count == 1);
  float f = args[0];
  return cosf(f);
}

float sqrt_impl(int count, float *args) {
  assert(count == 1);
  float f = args[0];
  return sqrtf(f);
}

float max_impl(int count, float *args) {
  assert(count == 2);
  float x = args[0];
  float y = args[1];
  return (x > y) ? x : y;
}

float min_impl(int count, float *args) {
  assert(count == 2);
  float x = args[0];
  float y = args[1];
  return (x < y) ? x : y;
}

BuiltinSlice get_builtins(void) {
  uint16_t count = 0;

  builtin_storage[count++] = init_builtin("sin", 1, sin_impl, "def sin(x) := _builtin_sin(x)");
  builtin_storage[count++] = init_builtin("cos", 1, cos_impl, "def cos(x) := _builtin_cos(x)");
  builtin_storage[count++] = init_builtin("sqrt", 1, sqrt_impl, "def sqrt(x) := _builtin_sqrt(x)");
  builtin_storage[count++] = init_builtin("min", 2, min_impl, "def min(x, y) := _builtin_min(x, y)");
  builtin_storage[count++] = init_builtin("max", 2, max_impl, "def max(x, y) := _builtin_max(x, y)");

  BuiltinSlice builtins = {
    .builtins = builtin_storage,
    .count = count,
  };

  return builtins;
}
