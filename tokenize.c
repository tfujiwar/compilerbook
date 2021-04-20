#include "9cc.h"

bool is_token_char(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->at = str;
  tok->str = substring(str, len);
  cur->next = tok;
  return tok;
}

Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (memcmp(p, "char", 4) == 0) {
      cur = new_token(TK_CHAR, cur, p, 4);
      p += 4;
      continue;
    }

    if (memcmp(p, "int", 3) == 0) {
      cur = new_token(TK_INT, cur, p, 3);
      p += 3;
      continue;
    }

    if (memcmp(p, "==", 2) == 0 || memcmp(p, "!=", 2) == 0 || memcmp(p, "<=", 2) == 0 || memcmp(p, ">=", 2) == 0) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>{}[]=;,*&", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      char* prev = p;
      int val = strtol(p, &p, 10);
      cur = new_token(TK_NUM, cur, prev, p - prev);
      cur->val = val;
      continue;
    }

    if (memcmp(p, "return", 6) == 0 && !is_token_char(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (memcmp(p, "if", 2) == 0 && !is_token_char(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if (memcmp(p, "else", 4) == 0 && !is_token_char(p[4])) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if (memcmp(p, "for", 3) == 0 && !is_token_char(p[3])) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }

    if (memcmp(p, "while", 5) == 0 && !is_token_char(p[5])) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if (memcmp(p, "sizeof", 6) == 0 && !is_token_char(p[6])) {
      cur = new_token(TK_SIZEOF, cur, p, 6);
      p += 6;
      continue;
    }

    if (is_token_char(*p)) {
      char* prev = p;
      while (is_token_char(*p)) p++;
      cur = new_token(TK_IDENT, cur, prev, p - prev);
      continue;
    }
    if (*p == '"') {
      char *prev = ++p;
      while (*p != '"' || *p == '\\') p++;
      cur = new_token(TK_STRING, cur, prev, p - prev);
      p++;
      continue;
    }

    error_at(p, "failed to tokenize");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
