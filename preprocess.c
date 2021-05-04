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

void replace_tokens(Token *begin, Token *end, Token *replace_begin, Token *replace_end) {
  begin->next = replace_begin;
  replace_end->next = end;
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
  Token *tok = &head;

  // For all tokens
  while(tok && tok->next && tok != until) {
    if (tok->next->kind != TK_IDENT) {
      tok = tok->next;
      continue;
    }

    // For all macros
    bool replaced = false;
    for (int i = 0; i < macros->keys->len; i++) {
      Macro *m = macros->vals->data[i];

      // Skip if macro is already applied
      if (m->used) continue;

      // Skip if macro doesn't match
      if (strcmp(tok->next->str, m->from->str) != 0) continue;

      Token *replace_begin = copy_tokens(m->to);

      // begin -> MACRO -> end
      Token *begin = tok;
      Token *end = tok->next->next;

      if (m->ty == FUNCTION) {
        replace_begin = replace_macro_params(tok->next, replace_begin, m);

        int depth = 0;
        end = tok->next->next->next;  // tok -> MACRO -> ( -> PARAM1

        for (; end; end = end->next) {
          if (strcmp(end->str, "(") == 0) depth++;
          else if (depth != 0 && strcmp(end->str, ")") == 0) depth--;
          else if (depth == 0 && strcmp(end->str, ")") == 0) break;
        }
        end = end->next;  // MACRO -> ( -> ... -> ... -> ) -> ...
      }

      Token *replace_end = replace_begin;
      while (replace_end->next->kind != TK_EOF) replace_end = replace_end->next;

      // Apply macros to replacing tokens
      m->used = true;
      replace_begin = apply_macros(replace_begin, replace_end);
      m->used = false;

      replace_end = replace_begin;
      while (replace_end->next->kind != TK_EOF) replace_end = replace_end->next;

      replace_tokens(begin, end, replace_begin, replace_end);
      tok = replace_end;

      replaced = true;
      break;
    }
    if (replaced) continue;
    tok = tok->next;
  }

  return head.next;
}
