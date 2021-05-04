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

      // Function-like macro
      if (*p == '(') {
        p++;
        macro->ty = FUNCTION;
        macro->params = new_vec();

        while (true) {
          while (*p == ' ') p++;
          char* prev = p;
          while (is_ident_char(*p)) p++;
          vec_push(macro->params, substring(prev, p - prev));

          while (*p == ' ') p++;
          if (*p == ')') {
            p++;
            break;
          }
          if (*p == ',') p++;
          else error_at(p, "failed to tokenize function-like macro");
        }
      } else {
        macro->ty = OBJECT;
      }

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

Token *replace_macro_params(Token *from, Token *to, Macro *macro) {
  from = copy_tokens(from);
  to = copy_tokens(to);

  Token *from_cur = from->next->next;
  if (!from_cur) error("failed to replace macro params");

  Token head;
  head.next = to;
  Token *to_cur = &head;

  for (int i = 0; i < macro->params->len; i++) {
    char *name = macro->params->data[i];

    // Move from_cur to the next param
    Token *param_start = from_cur;
    int depth = 0;
    while (from_cur->next) {
      if (strcmp(from_cur->next->str, "(") == 0) depth++;
      else if (depth != 0 && strcmp(from_cur->next->str, ")") == 0) depth--;
      else if (depth == 0 && (strcmp(from_cur->next->str, ")") == 0) || (strcmp(from_cur->next->str, ",") == 0)) break;
      from_cur = from_cur->next;
    }
    Token *param_end = from_cur;
    from_cur = from_cur->next->next;

    // Replace the param
    Token *to_cur = &head;
    while (to_cur->next) {
      if (strcmp(to_cur->next->str, name) == 0) {
        param_end->next = to_cur->next->next;
        to_cur->next = param_start;
        break;
      }
      to_cur = to_cur->next;
    }
  }
  return head.next;
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

          if (m->ty == FUNCTION) {
            to = replace_macro_params(tok, to, m);
            int depth = 0;
            tok = tok->next->next;  // consume (
            while (tok) {
              if (strcmp(tok->str, "(") == 0) depth++;
              else if (depth != 0 && strcmp(tok->str, ")") == 0) depth--;
              else if (depth == 0 && strcmp(tok->str, ")") == 0) break;
              tok = tok->next;
            }
          }

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
