#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>

#pragma once

enum TokenType {
  TKN_PLUS,
  TKN_MULT,
  TKN_DIV,
  TKN_MINUS,
  TKN_NUMBER,
  TKN_LPAREN,
  TKN_RPAREN,
};

typedef struct {
  enum TokenType type;
  float num;
} Token;

void token_print(Token *t) {
  switch (t->type) {
  case TKN_PLUS:
    printf("PLUS");
    break;
  case TKN_MINUS:
    printf("MINUS");
    break;
  case TKN_MULT:
    printf("MULT");
    break;
  case TKN_DIV:
    printf("DIV");
     break;
  case TKN_LPAREN:
    printf("LPAREN");
    break;
  case TKN_RPAREN:
    printf("RPAREN");
    break;
  case TKN_NUMBER:
    printf("NUMBER(%g)", t->num);
    break;
  }
}

bool token_equals(Token *a, Token *b) {
  if (a->type == TKN_NUMBER) {
    return b->type == TKN_NUMBER && fabs(a->num - b->num) < 0.00001;
  }

  return a->type == b->type;
}

#define MAX_TOKENS 1000
#define MAX_NUM_DIGITS 36

uint16_t token_idx;

bool is_num(char c) {
  return ('0' <= c && c <= '9') || c == '.';
}

// return the length of consumed char
int consume_num(const char *str, Token *tokens) {
  float num;

  char num_buf[MAX_NUM_DIGITS];

  num_buf[0] = *str++;

  uint8_t len = 1;

  while (is_num(*str) || *str == '.') {
    num_buf[len++] = *str;
    str++;
  }

  num_buf[len] = '\0';

  num = strtof(num_buf, NULL);

  tokens[token_idx++] = (Token){.type = TKN_NUMBER, .num = num};
  return len - 1;
}

// returns the total token count
int run_lexer(const char *str, Token *tokens) {
  token_idx = 0;

  for (;*str != '\0'; str++) {
    if (is_num(*str)) {
      str += consume_num(str, tokens);
    }

    switch (*str) {
    case '+':
      tokens[token_idx++] = (Token){.type = TKN_PLUS};
      continue;
    case '*':
      tokens[token_idx++] = (Token){.type = TKN_MULT};
      continue;
    case '-':
      tokens[token_idx++] = (Token){.type = TKN_MINUS};
      continue;
    case '/':
      tokens[token_idx++] = (Token){.type = TKN_DIV};
      continue;
    case '(':
      tokens[token_idx++] = (Token){.type = TKN_LPAREN};
      continue;
    case ')':
      tokens[token_idx++] = (Token){.type = TKN_RPAREN};
      continue;
    }
  }

  return token_idx;
}
