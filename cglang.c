#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

enum token_type {
  PLUS,
  MULT,
  DIV,
  MINUS,
  NUMBER,
};

typedef struct {
  enum token_type type;
  float num;
} Token;

#define MAX_TOKENS 1000
#define MAX_NUM_DIGITS 10

bool is_num(char c) {
  return ('0' <= c && c <= '9') || c == '.';
}

void run_lexer(char *str, Token *tokens) {
  int token_idx = 0;
  char num_buf[MAX_NUM_DIGITS];

  for (;*str != '\0'; str++) {
    if (*str == '+') {
      tokens[token_idx++] = (Token){.type = PLUS};
      continue;
    }
    if (*str == '*') {
      tokens[token_idx++] = (Token){.type = MULT};
      continue;
    }
    if (*str == '-') {
      tokens[token_idx++] = (Token){.type = MINUS};
      continue;
    }
    if (*str == '/') {
      tokens[token_idx++] = (Token){.type = DIV};
      continue;
    }

    if (is_num(*str)) {
      int num_i = 0;
      while (is_num(*str)) {
        num_buf[num_i++] = *str;
        str++;
      }

       = '\0';
      str--;
      tokens[token_idx++] = (Token){.type = NUMBER, .num = strtof(num_buf, num_buf[num_i])};
    }
  }
}
