#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#pragma once

enum TokenType {
  TKN_PLUS,
  TKN_MULT,
  TKN_DIV,
  TKN_MINUS,
  TKN_NUMBER,
};

typedef struct {
  enum TokenType type;
  float num;
} Token;

bool token_equals(Token *a, Token *b) {
  if (a->type == TKN_NUMBER) {
    return b->type == TKN_NUMBER && fabs(a->num - b->num) < 0.00001;
  }

  return a->type == b->type;
}

#define MAX_TOKENS 1000
#define MAX_NUM_DIGITS 36

bool is_num(char c) {
  return ('0' <= c && c <= '9') || c == '.';
}

int get_num(const char *str, float *out) {
    // TODO: This version requires minus operator to have space after
    if (*str == '-' && *(str + 1) == ' ') {
      return 0;
    }
      
    char num_buf[MAX_NUM_DIGITS];

    num_buf[0] = *str++;

    int num_i = 1;

    while (is_num(*str) || *str == '.') {
      num_buf[num_i++] = *str;
      str++;
    }

    num_buf[num_i] = '\0';

    *out = strtof(num_buf, NULL);

    return num_i;
}

// returns the total token count
int run_lexer(const char *str, Token *tokens) {
  int token_idx = 0;

  for (;*str != '\0'; str++) {
    if (is_num(*str) || (*str == '-')) {
      float num;
      int len = get_num(str, &num);

      if (len > 0) {
        tokens[token_idx++] = (Token){.type = TKN_NUMBER, .num = num};
        str += len - 1;
        continue;
      }
    }

    if (*str == '+') {
      tokens[token_idx++] = (Token){.type = TKN_PLUS};
      continue;
    }
    if (*str == '*') {
      tokens[token_idx++] = (Token){.type = TKN_MULT};
      continue;
    }
    if (*str == '-') {
      tokens[token_idx++] = (Token){.type = TKN_MINUS};
      continue;
    }
    if (*str == '/') {
      tokens[token_idx++] = (Token){.type = TKN_DIV};
      continue;
    }
  }

  return token_idx;
}
