#include "mycc.h"

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

Token *apply_macros() {
  Token head;
  head.next = token;
  Token *prev = &head;
  Token *tok = token;
  while (tok) {
    if (tok->kind == TK_IDENT) {
      for (int i = 0; i < macros->keys->len; i++) {
        Macro *m = macros->vals->data[i];
        if (strcmp(tok->str, m->from->str) == 0) {
          Token *end = m->to;
          while (end->next && end->next->kind != TK_EOF) end = end->next;
          prev->next = m->to;
          end->next = tok->next;

          prev = end;
          tok = end->next;
          continue;
        }
      }
    }
    prev = tok;
    tok = tok->next;
  }

  return head.next;
}