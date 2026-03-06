#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>

#pragma once

enum TokenType {
  TKN_PLUS,
  TKN_MULT,
  TKN_DIV,
  TKN_MINUS,
  TKN_NUMBER,
  TKN_LPAREN,
  TKN_RPAREN,
  TKN_VAR,
  TKN_ASSIGNMENT,
  TKN_DEF,
  TKN_SEMICOLON,
};

typedef struct {
  enum TokenType type;
  float num;
  char *var;
} Token;

void token_print(const Token *t) {
  switch (t->type) {
  case TKN_PLUS:       printf("PLUS");               break;
  case TKN_MINUS:      printf("MINUS");              break;
  case TKN_MULT:       printf("MULT");               break;
  case TKN_DIV:        printf("DIV");                break;
  case TKN_LPAREN:     printf("LPAREN");             break;
  case TKN_RPAREN:     printf("RPAREN");             break;
  case TKN_NUMBER:     printf("NUMBER(%g)", t->num); break;
  case TKN_VAR:        printf("VAR(%s)", t->var);    break;
  case TKN_ASSIGNMENT: printf("ASSIGNMENT");         break;
  case TKN_DEF:        printf("DEF");                break;
  case TKN_SEMICOLON:  printf("SEMICOLON");          break;
  }
}

void tokens_print(const Token *tokens, uint16_t token_count) {
  for (uint16_t i = 0; i < token_count; i ++) {
    token_print(&tokens[i]);
    printf("\n");
  }
}

bool token_equals(const Token *a, const Token *b) {
  if (a->type == TKN_NUMBER) {
    return b->type == TKN_NUMBER && fabs(a->num - b->num) < 0.00001;
  }

  if (a->type == TKN_VAR) {
    return b->type == TKN_VAR
           && (strcmp(a->var, b->var) == 0);
  }

  return a->type == b->type;
}

// max of uint16_t
#define MAX_TOKENS 65535
#define MAX_NUM_DIGITS 36
#define MAX_VAR_CHAR 512

uint16_t token_idx;

bool is_num(char c) {
  return ('0' <= c && c <= '9') || c == '.';
}

// return the length of consumed char
uint16_t consume_num(const char *str, Token *tokens) {
  if (!is_num(*str)) return 0;

  float num;

  char num_buf[MAX_NUM_DIGITS];

  num_buf[0] = *str++;

  uint16_t len = 1;

  while (is_num(*str) || *str == '.') {
    num_buf[len++] = *str;
    str++;
  }

  num_buf[len] = '\0';

  num = strtof(num_buf, NULL);

  tokens[token_idx++] = (Token){.type = TKN_NUMBER, .num = num};
  return len;
}

uint16_t consume_assignment(const char *str, Token *tokens) {
  if (strncmp(str, ":=", 2) == 0) {
    tokens[token_idx++] = (Token){.type = TKN_ASSIGNMENT};

    return 2;
  }
  return 0;
}

uint16_t consume_def(const char *str, Token *tokens) {
  if (strncmp(str, "def", 3) == 0) {
    tokens[token_idx++] = (Token){.type = TKN_DEF};

    return 3;
  }
  return 0;
}

bool is_var_char(char c) {
  return isalpha(c) || c == '_';
}

uint16_t consume_var(const char *str, Token *tokens) {
  if (!is_var_char(*str)) return 0;

  char *var;

  char var_buf[MAX_VAR_CHAR];

  var_buf[0] = *str++;

  uint16_t len = 1;

  while (is_var_char(*str)) {
    var_buf[len++] = *str;
    str++;
  }

  var_buf[len] = '\0';

  var = malloc(sizeof(char) * len);

  memcpy(var, var_buf, len);

  tokens[token_idx++] = (Token){.type = TKN_VAR, .var = var};

  return len;
}

// returns the total token count
int run_lexer(const char *str, Token *tokens) {
  token_idx = 0;

  while (*str != '\0') {
    uint16_t len = 0;
    if ((len = consume_num(str, tokens))) {
     str += len;
     continue;
    }

    if ((len = consume_assignment(str, tokens))) {
      str += len;
      continue;
    }

    if ((len = consume_def(str, tokens))) {
      str += len;
      continue;
    }

    if ((len = consume_var(str, tokens))) {
      str += len;
      continue;
    }

    switch (*str) {
    case '+':
      tokens[token_idx++] = (Token){.type = TKN_PLUS};
      str++;
      continue;
    case '*':
      tokens[token_idx++] = (Token){.type = TKN_MULT};
      str++;
      continue;
    case '-':
      tokens[token_idx++] = (Token){.type = TKN_MINUS};
      str++;
      continue;
    case '/':
      tokens[token_idx++] = (Token){.type = TKN_DIV};
      str++;
      continue;
    case '(':
      tokens[token_idx++] = (Token){.type = TKN_LPAREN};
      str++;
      continue;
    case ')':
      tokens[token_idx++] = (Token){.type = TKN_RPAREN};
      str++;
      continue;
    case ';':
      tokens[token_idx++] = (Token){.type = TKN_SEMICOLON};
      str++;
      continue;
    }

    // unknown token, or space
    str++;
  }

  return token_idx;
}
