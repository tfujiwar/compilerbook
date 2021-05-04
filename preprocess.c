#include "mycc.h"

Token *copy_tokens(Token *token) {
  Token head;
  head.next = NULL;

  Token *src = token;
  Token *prev = &head;

  while (src) {
    Token *dst = calloc(1, sizeof(Token));
    dst->kind = src->kind;
    dst->at = src->at;
    dst->str = src->str;
    dst->val = src->val;
    prev->next = dst;

    src = src->next;
    prev = prev->next;
  }

  return head.next;
}

void preprocess(char *user_input) {
  char *p = user_input;
  Token head;
  head.next = NULL;

  while (*p) {
    if (*p != '#') {
      char *eol = strchr(p, '\n');
      if (!eol) return;
      p = eol + 1;
      continue;
    }

    p++;
    while (*p == ' ') p++;

    if (strncmp(p, "define", 6) == 0 && *(p+6) == ' ') {
      p += 6;
      while (*p == ' ') p++;

      Macro *macro = calloc(1, sizeof(Macro));

      char* prev = p;
      while (is_ident_char(*p)) p++;
      macro->from = new_token(TK_IDENT, &head, prev, p - prev);
      macro->name = macro->from->str;

      while (isspace(*p)) p++;

      char *eol = strchr(p, '\n');
      if (!eol) return;
      macro->to = tokenize(substring(p, eol - p));

      map_put(macros, macro->name ,macro);

      p = eol + 1;
      continue;
    }

    error_at(p, "failed to tokenize macro");
  }
}

Token *apply_macros(Token *token, Token *until) {
  Token head;
  head.next = token;
  Token *prev = &head;
  Token *tok = token;
  while (tok) {
    if (tok->kind == TK_IDENT) {
      bool replaced = false;

      for (int i = 0; i < macros->keys->len; i++) {
        Macro *m = macros->vals->data[i];
        if (m->used) continue;

        if (strcmp(tok->str, m->from->str) == 0) {
          Token *to = copy_tokens(m->to);
          Token *end = to;
          while (end->next && end->next->kind != TK_EOF) end = end->next;

          // Recurse
          m->used = true;
          to = apply_macros(to, end->next);
          m->used = false;

          // Replace
          prev->next = to;
          end->next = tok->next;

          prev = end;
          tok = end->next;

          replaced = true;
          break;
        }
      }
      if (replaced) continue;
    }
    prev = tok;
    tok = tok->next;
  }

  return head.next;
}
