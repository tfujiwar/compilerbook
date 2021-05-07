#include "mycc.h"

bool is_ident_char(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

Token *new_token(TokenKind kind, Token *cur, Source *src, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->src = src;
  tok->at = str;
  tok->str = substring(str, len);
  cur->next = tok;
  return tok;
}

Token *tokenize(Source *src, char **p, bool is_macro) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  char *reserved[] = {
    "++", "--",
    "==", "!=", "<=", ">=",
    "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "|=", "^=",
    "<<", ">>", "&&", "||",
    "->",
    "+", "-", "*", "/", "%",
    "(", ")", "<", ">", "{", "}", "[", "]",
    "=", ";", ",", ".",
    "&", "|", "^", "~", "!", "?", ":",
  };

  while (**p) {
    if (is_macro && **p == '\n') break;
    if (!is_macro && **p == '\n' && *(*p+1) == '#') break;

    if (isspace(**p)) {
      (*p)++;
      continue;
    }

    if (**p == '\\' && *(*p+1) == '\n') {
      *p += 2;
      continue;
    }

    if (memcmp(*p, "##", 2) == 0) {
      cur = new_token(TK_CONCAT, cur, src, *p, 2);
      *p += 2;
      continue;
    }

    if (**p == '#') {
      cur = new_token(TK_STRINGIFY, cur, src, *p, 1);
      *p += 1;
      continue;
    }

    if (memcmp(*p, "...", 3) == 0) {
      cur = new_token(TK_ELLIPSIS, cur, src, *p, 3);
      *p += 3;
      continue;
    }

    if (memcmp(*p, "//", 2) == 0) {
      *p += 2;
      while (**p != '\n' && **p != '\0') (*p)++;
      continue;
    }

    if (memcmp(*p, "/*", 2) == 0) {
      *p += 2;
      while (memcmp(*p, "*/", 2) != 0 && **p != '\0') (*p)++;
      if (**p) *p += 2;
      continue;
    }

    bool found = false;
    for (int i = 0; i < sizeof(reserved) / sizeof(char*); i++) {
      if (memcmp(*p, reserved[i], strlen(reserved[i])) == 0) {
        cur = new_token(TK_RESERVED, cur, src, *p, strlen(reserved[i]));
        *p += strlen(reserved[i]);
        found = true;
        break;
      }
    }
    if (found) continue;

    if (memcmp(*p, "void", 4) == 0) {
      cur = new_token(TK_VOID, cur, src, *p, 4);
      *p += 4;
      continue;
    }

    if (memcmp(*p, "char", 4) == 0) {
      cur = new_token(TK_CHAR, cur, src, *p, 4);
      *p += 4;
      continue;
    }

    if (memcmp(*p, "int", 3) == 0) {
      cur = new_token(TK_INT, cur, src, *p, 3);
      *p += 3;
      continue;
    }

    if (memcmp(*p, "typedef", 7) == 0) {
      cur = new_token(TK_TYPEDEF, cur, src, *p, 7);
      *p += 7;
      continue;
    }

    if (isdigit(**p)) {
      char* prev = *p;
      int val = strtol(*p, p, 10);
      cur = new_token(TK_NUM, cur, src, prev, *p - prev);
      cur->val = val;
      if (**p == 'l' || **p == 'L') (*p)++;  // TODO
      continue;
    }

    if (memcmp(*p, "return", 6) == 0 && !is_ident_char((*p)[6])) {
      cur = new_token(TK_RETURN, cur, src, *p, 6);
      *p += 6;
      continue;
    }

    if (memcmp(*p, "if", 2) == 0 && !is_ident_char((*p)[2])) {
      cur = new_token(TK_IF, cur, src, *p, 2);
      *p += 2;
      continue;
    }

    if (memcmp(*p, "else", 4) == 0 && !is_ident_char((*p)[4])) {
      cur = new_token(TK_ELSE, cur, src, *p, 4);
      *p += 4;
      continue;
    }

    if (memcmp(*p, "for", 3) == 0 && !is_ident_char((*p)[3])) {
      cur = new_token(TK_FOR, cur, src, *p, 3);
      *p += 3;
      continue;
    }

    if (memcmp(*p, "while", 5) == 0 && !is_ident_char((*p)[5])) {
      cur = new_token(TK_WHILE, cur, src, *p, 5);
      *p += 5;
      continue;
    }

    if (memcmp(*p, "do", 2) == 0 && !is_ident_char((*p)[2])) {
      cur = new_token(TK_DO, cur, src, *p, 2);
      *p += 2;
      continue;
    }

    if (memcmp(*p, "switch", 6) == 0 && !is_ident_char((*p)[6])) {
      cur = new_token(TK_SWITCH, cur, src, *p, 6);
      *p += 6;
      continue;
    }

    if (memcmp(*p, "case", 4) == 0 && !is_ident_char((*p)[4])) {
      cur = new_token(TK_CASE, cur, src, *p, 4);
      *p += 4;
      continue;
    }

    if (memcmp(*p, "default", 7) == 0 && !is_ident_char((*p)[7])) {
      cur = new_token(TK_DEFAULT, cur, src, *p, 7);
      *p += 7;
      continue;
    }

    if (memcmp(*p, "break", 5) == 0 && !is_ident_char((*p)[5])) {
      cur = new_token(TK_BREAK, cur, src, *p, 5);
      *p += 5;
      continue;
    }

    if (memcmp(*p, "sizeof", 6) == 0 && !is_ident_char((*p)[6])) {
      cur = new_token(TK_SIZEOF, cur, src, *p, 6);
      *p += 6;
      continue;
    }

    if (memcmp(*p, "defined", 7) == 0 && !is_ident_char((*p)[7])) {
      cur = new_token(TK_DEFINED, cur, src, *p, 7);
      *p += 7;
      continue;
    }

    if (memcmp(*p, "struct", 6) == 0 && !is_ident_char((*p)[6])) {
      cur = new_token(TK_STRUCT, cur, src, *p, 6);
      *p += 6;
      continue;
    }

    if (memcmp(*p, "enum", 4) == 0 && !is_ident_char((*p)[4])) {
      cur = new_token(TK_ENUM, cur, src, *p, 4);
      *p += 4;
      continue;
    }

    if (memcmp(*p, "extern", 6) == 0 && !is_ident_char((*p)[6])) {
      cur = new_token(TK_EXTERN, cur, src, *p, 6);
      *p += 6;
      continue;
    }

    if (is_ident_char(**p)) {
      char* prev = *p;
      while (is_ident_char(**p)) (*p)++;
      cur = new_token(TK_IDENT, cur, src, prev, *p - prev);
      continue;
    }

    if (**p == '"') {
      char *prev = ++(*p);
      while (**p != '"' || *(*p-1) == '\\') (*p)++;
      cur = new_token(TK_STRING, cur, src, prev, *p - prev);
      (*p)++;
      continue;
    }

    if (**p == '\'' ) {
      cur = new_token(TK_NUM, cur, src, *p + 1, 1);
      cur->val = *(*p + 1);
      *p += 3;
      continue;
    }

    error_at(src, *p, "failed to tokenize");
  }

  new_token(TK_EOF, cur, src, *p, 0);

  (*p)++;  // read newline
  return head.next;
}
