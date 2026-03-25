#include <stdbool.h>
#define UNITY_BUILD

#include <stdio.h>


#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "lexer.c"
#include "parser.c"
#include "typechecker.c"
#include "interpreter.c"

void print_graph(Function f);
int read_file(FILE *in, char *buf);
void usage(void);

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
    // TODO: this works only when the program is one-liner
    fprintf(stderr, "%s\n", program);
    fprintf(stderr, "%*s^\n", lexer_err.position, "");
    fprintf(stderr, "Lexer Error: %s\n", lexer_err.message);
    return 1;
  }

  AST *ast = run_parser(tokens, token_count, &parser_err);
  if (ast == NULL) {
    fprintf(stderr, "%s\n", program);
    fprintf(stderr, "%*s^\n", parser_err.position, "");
    fprintf(stderr, "Parser Error: %s\n", parser_err.message);
    return 1;
  }

  if (check_type) {
    TypeError type_err = type_error_init(error_message_buf);
    Type *res = run_typechecker(ast, &type_err);

    if (res == NULL) {
      fprintf(stderr, "TypeError\n");
      return 1;
    }
  }

  Value result = interpret(ast);

  if (result.type == VAL_NUM) {
    printf("%g\n", result.num);
  } else if (result.type == VAL_FUNC) {
    print_graph(result.func);
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

void print_graph(Function f) {
  FILE *gnuplot = popen("gnuplot -persist", "w");

  if (gnuplot == NULL) {
    fprintf(stderr, "failed to open gnuplot.\n");
    exit(1);
  }

  fprintf(gnuplot, "plot '-' with linespoints title 'function<%s>'\n", f.name);

  for (int16_t i = -10; i <= 10; i++) {
    Assignment *assignment = NULL;
    shput(assignment, f.args, (float)i);
    float num = expectNumber(interpretExpression(f.body, NULL, assignment));
    fprintf(gnuplot, "%d %g\n", i, num);
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
