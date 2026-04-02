#include <stdbool.h>
#include <stdint.h>
#define UNITY_BUILD

#include <stdio.h>


#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "lexer.c"
#include "parser.c"
#include "typechecker.c"
#include "interpreter.c"

void print_graph(Function f, int16_t min, int16_t max, DefMap *dm, BuiltinMap *bm);
int read_file(FILE *in, char *buf);
void usage(void);

// TODO: this works only when the program is one-liner
#define PRINT_ERR(program, err_name, err) \
      fprintf(stderr, "%s\n", program); \
      fprintf(stderr, "%*s^\n", err.position, ""); \
      fprintf(stderr, "%s: %s\n", err_name, err.message); \
      exit(1);

bool check_type = true;

int main (int argc, char *argv[]) {
  char *program;

  for (int i = 1; i < argc; i++) { 
    if (strncmp(argv[i], "-e", 2) == 0) {
      if (i+1 >= argc) {
        usage();
        exit(1);
      }
      program = argv[++i];
    } else if (strncmp(argv[i], "-f", 2) == 0) {
      if (i+1 >= argc) {
        usage();
        exit(1);
      }
      FILE *f = fopen(argv[++i], "r");

      if (f == NULL) {
        fprintf(stderr, "Couldn't open file: %s", argv[2]);
        exit(1);
      }

      char buf[2048];

      read_file(f, buf);

      program = buf;

      fclose(f);
    } else if (strncmp(argv[i], "--no-typecheck", 2) == 0) {
      check_type= false;
    } else {
      fprintf(stderr, "Unknown flag: %s", argv[i]);
      usage();
      exit(1);
    }
  }

  Token tokens[MAX_TOKENS];

  char error_message_buf[256]; 
  LexerError lexer_err = lexer_error_init(error_message_buf);
  ParserError parser_err = parser_error_init(error_message_buf);

  int token_count = run_lexer(program, tokens, &lexer_err);

  if (token_count == LEXER_ERROR) {
    PRINT_ERR(program, "Lexer Error", lexer_err);
  }

  AST *ast = run_parser(tokens, token_count, &parser_err);
  if (ast == NULL) {
      PRINT_ERR(program, "Parse Error", parser_err);
  }

  if (check_type) {
    TypeError type_err = type_error_init(error_message_buf);
    Type *res = run_typechecker(ast, &type_err);

    if (res == NULL) {
      PRINT_ERR(program, "Type Error", type_err);
    }
  }

  InterpretResult result = interpret(ast);

  Value result_val = result.v;

  if (result_val.type == VAL_NUM) {
    printf("%g\n", result_val.num);
  } else if (result_val.type == VAL_FUNC) {
    print_graph(result_val.func, -10, 10, result.dm, result.bm);
  }

  return 0;
}

int read_file(FILE *in, char *buf) {
  buf[0] = '\0';

  char line [1024];
  uint16_t len = 0;

  while (fgets(line, 1024, in)) {
    len = strlen(line);
    strncat(buf, line, len);
  }

  if (feof(in) == 0) {
    fprintf(stderr, "Error happend during reading file");
    exit(1);
  }

  return 1;
}

void print_graph(Function f, int16_t min, int16_t max, DefMap *dm, BuiltinMap *bm) {
  if (f.args_len > 1) {
    fprintf(stderr, "Printing graph is only available for functions with 1 argument.\n");
    exit(1);
  }

  FILE *gnuplot = popen("gnuplot -persist", "w");

  if (gnuplot == NULL) {
    fprintf(stderr, "failed to open gnuplot.\n");
    fclose(gnuplot);
    exit(1);
  }

  fprintf(gnuplot, "plot '-' with linespoints title 'function<%s>'\n", f.name);

  if (max <= min) {
    fprintf(stderr, "max (%d) should be more than min (%d).\n", max, min);
    fclose(gnuplot);
    exit(1);
  }

  uint16_t plots = 100; // how many points to plot
  float step = (max - min) / (float)plots;

  for (uint16_t i = 0; i <= plots; i++) {
    float x = min + i * step;

    Assignment *assignment = NULL;
    shput(assignment, f.args[0], (float)x);
    float num = expectNumber(interpretExpression(f.body, dm, assignment, bm));
    fprintf(gnuplot, "%g %g\n", x, num);
  }

  fprintf(gnuplot, "e\n");

  pclose(gnuplot);
}

void usage(void) {
    printf(
      "Usage: cglang\n"
      "-e [program]\n"
      "-f [input file]\n"
      "--no-typecheck\n"
    );
}
