#include "mycc.h"

typedef struct IfBlock IfBlock;

struct IfBlock {
  bool active;
  bool true_found;
  IfBlock* parent;
};

bool file_exists(char *filename) {
  FILE *file = fopen(filename, "r");
  if (file) {
    fclose(file);
    return true;
  }
  return false;
}

bool is_space(char c) {
  return (c == ' ') || (c == '\t');
}

char *skip_spaces(char *cur) {
  while (is_space(*cur)) cur++;
  return cur;
}

IfBlock *new_if_block(IfBlock *parent) {
  IfBlock *if_block = calloc(1, sizeof(IfBlock));
  if_block->parent = parent;
  return if_block;
}

Source *new_source(Source *parent, char* filename) {
  Source *source = calloc(1, sizeof(Source));
  source->filename = filename;
  source->parent = parent;
  source->head = read_file(filename);
  source->cur = source->head;
  return source;
}

char *next_ident(char *cur) {
  cur = skip_spaces(cur);
  char* begin = cur;
  while (is_ident_char(*cur)) cur++;
  return substring(begin, cur - begin);
}

Token *copy_tokens(Token *token) {
  Token head;
  head.next = NULL;

  Token *src = token;
  Token *prev = &head;

  while (src) {
    Token *dst = calloc(1, sizeof(Token));
    dst->kind = src->kind;
    dst->src = src->src;
    dst->at = src->at;
    dst->str = src->str;
    dst->val = src->val;
    prev->next = dst;

    src = src->next;
    prev = prev->next;
  }

  return head.next;
}

void replace_tokens(Token *begin, Token *end, Token *replace_begin, Token *replace_end) {
  begin->next = replace_begin;
  replace_end->next = end;
}

Token *replace_undefined(Token *token) {
  Token head;
  head.next = token;
  Token *tok = &head;

  // For all tokens
  while (tok->next) {
    if (tok->next->kind == TK_IDENT) {
      Token *ident = tok->next;
      Token *replace = new_token(TK_NUM, tok, tok->src, "0", 1);
      replace->val = 0;
      replace->at = ident->at;
      replace->next = ident->next;
    }
    tok = tok->next;
  }

  return head.next;
}

int eval_node(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    return node->val;

  case ND_ADD:
    return eval_node(node->lhs) + eval_node(node->rhs);

  case ND_SUB:
    return eval_node(node->lhs) - eval_node(node->rhs);

  case ND_MUL:
    return eval_node(node->lhs) + eval_node(node->rhs);

  case ND_DIV:
    return eval_node(node->lhs) / eval_node(node->rhs);

  case ND_MOD:
    return eval_node(node->lhs) % eval_node(node->rhs);

  case ND_EQ:
    return eval_node(node->lhs) == eval_node(node->rhs);

  case ND_NE:
    return eval_node(node->lhs) != eval_node(node->rhs);

  case ND_LE:
    return eval_node(node->lhs) <= eval_node(node->rhs);

  case ND_LT:
    return eval_node(node->lhs) < eval_node(node->rhs);

  case ND_BITWISE_AND:
    return eval_node(node->lhs) & eval_node(node->rhs);

  case ND_BITWISE_OR:
    return eval_node(node->lhs) | eval_node(node->rhs);

  case ND_BITWISE_XOR:
    return eval_node(node->lhs) ^ eval_node(node->rhs);

  case ND_SHIFT_LEFT:
    return eval_node(node->lhs) << eval_node(node->rhs);

  case ND_SHIFT_RIGHT:
    return eval_node(node->lhs) >> eval_node(node->rhs);

  case ND_LOGICAL_AND:
    return eval_node(node->lhs) && eval_node(node->rhs);

  case ND_LOGICAL_OR:
    return eval_node(node->lhs) || eval_node(node->rhs);

  case ND_CONDITIONAL:
    return eval_node(node->cond) ? eval_node(node->body) : eval_node(node->els);

  default:
    error("not supported operator in macro conditions");
  }
}

Token *replace_defined(Token *token) {
  Token head;
  head.next = token;
  Token *cur = &head;
  while (cur->next) {
    if (cur->next->kind != TK_DEFINED) {
      cur = cur->next;
      continue;
    }

    // move cur to "defined"
    Token *begin = cur;
    cur = cur->next;

    Token *end;
    char *name;

    if (cur->next->kind == TK_IDENT) {
      name = cur->next->str;
      end = cur->next->next;  // next to macro_name
    } else {
      name = cur->next->next->str;
      end = cur->next->next->next->next;  // next to ")"
    }

    Token *replace;
    if (map_get(macros, name)) {
      Token dummy;
      replace = new_token(TK_NUM, &dummy, begin->src, "1", 1);
      replace->val = 1;
      replace->at = begin->at;
    } else {
      Token dummy;
      replace = new_token(TK_NUM, &dummy, begin->src, "0", 1);
      replace->val = 0;
      replace->at = begin->at;
    }

    replace_tokens(begin, end, replace, replace);
    cur = replace;
  }
  return head.next;
}

Token* preprocess(Source *src) {
  Vector *include_paths = new_vec();
  vec_push(include_paths, "/usr/local/lib/gcc/x86_64-linux-gnu/10.2.0/include");
  vec_push(include_paths, "/usr/local/include");
  vec_push(include_paths, "/usr/local/lib/gcc/x86_64-linux-gnu/10.2.0/include-fixed");
  vec_push(include_paths, "/usr/include/x86_64-linux-gnu");
  vec_push(include_paths, "/usr/include");

  char *p = src->head;

  Token head;
  head.next = NULL;
  Token *token_cur = &head;

  bool output_enabled = true;
  IfBlock *if_block = NULL;
  Source *source = src;

  while (true) {
    // Return to parent source
    if (!*p) {
      if (!source->parent) break;
      source = source->parent;
      p = source->cur;
      continue;
    }

    // Not a macro directive
    if (*p != '#') {
      if (output_enabled) {
        Token *t = tokenize(source, &p);
        token_cur->next = apply_macros(t, NULL);
        while (token_cur->next->kind != TK_EOF) {
          token_cur = token_cur->next;
        }
      } else {
        char *eol = strchr(p, '\n');
        if (!eol) break;
        p = eol + 1;
      }

      continue;
    }

    // Read # and spaces
    p++;
    p = skip_spaces(p);

    // Include directive
    if (strncmp(p, "include", 7) == 0 && (is_space(*(p+7)) || *(p+7) == '<' || *(p+7) == '"')) {
      p += 7;
      p = skip_spaces(p);

      char *filename = calloc(256, sizeof(char));

      if (*p == '<') {
        p++;
        char *end = strchr(p, '>');
        bool found = false;
        for (int i = 0; i < include_paths->len; i++) {
          sprintf(filename, "%s/%s", include_paths->data[i], substring(p, end - p));
          if (file_exists(filename)) {
            found = true;
            break;
          }
        }
        if (!found) {
          // TODO
          debug("source not found: %s", substring(p, end - p));
          char *eol = strchr(p, '\n');
          p = eol + 1;
          continue;
        }

      } else if (*p == '"') {
        p++;
        char *end = strchr(p, '"');
        sprintf(filename, "%s", substring(p, end - p));

      } else {
        error("failed to parse include directive");
      }

      char *eol = strchr(p, '\n');
      p = eol + 1;
      source->cur = p;

      source = new_source(source, filename);
      p = source->cur;

      continue;
    }

    // Define directive
    if (strncmp(p, "define", 6) == 0 && is_space(*(p+6))) {
      p += 6;
      p = skip_spaces(p);

      Macro *macro = calloc(1, sizeof(Macro));

      char* prev = p;
      while (is_ident_char(*p)) p++;
      Token dummy;
      macro->from = new_token(TK_IDENT, &dummy, source, prev, p - prev);
      macro->name = macro->from->str;

      // Function-like macro
      if (*p == '(') {
        p++;
        macro->ty = FUNCTION;
        macro->params = new_vec();

        while (true) {
          p = skip_spaces(p);
          char* prev = p;
          while (is_ident_char(*p)) p++;
          vec_push(macro->params, substring(prev, p - prev));

          p = skip_spaces(p);
          if (*p == ')') {
            p++;
            break;
          }
          if (*p == ',') p++;
          else error_at(source, p, "failed to tokenize function-like macro");
        }
      } else {
        macro->ty = OBJECT;
      }

      char *eol = strchr(p, '\n');
      if (!eol) break;
      macro->to = tokenize(source, &p);

      map_put(macros, macro->name ,macro);

      continue;
    }

    // undef directive
    if (strncmp(p, "undef", 5) == 0 && is_space(*(p+5))) {
      p += 5;
      p = skip_spaces(p);

      char *name = next_ident(p);
      if (map_get(macros, name)) map_delete(macros, name);

      char *eol = strchr(p, '\n');
      if (!eol) break;

      p = eol + 1;
      continue;
    }

    // if directive
    if (strncmp(p, "if", 2) == 0 && is_space(*(p+2))) {
      p += 2;
      if_block = new_if_block(if_block);

      char *eol = strchr(p, '\n');
      if (!eol) break;

      if (output_enabled) {
        if_block->active = true;

        token = tokenize(source, &p);
        token = replace_defined(token);
        token = apply_macros(token, NULL);
        token = replace_undefined(token);
        Node *node = conditional();

        if (eval_node(node)) {
          if_block->true_found = true;
          output_enabled = true;
        } else {
          if_block->true_found = false;
          output_enabled = false;
        }

      } else {
        if_block->active = false;
      }

      continue;
    }

    // elif directive
    if (strncmp(p, "elif", 4) == 0 && is_space(*(p+4))) {
      if (!if_block) error_at(source, p, "not in if macro");

      p += 4;
      char *eol = strchr(p, '\n');
      if (!eol) break;

      if (if_block->active) {
        if (!if_block->true_found) {
          token = tokenize(source, &p);
          token = replace_defined(token);
          token = apply_macros(token, NULL);
          token = replace_undefined(token);
          Node *node = conditional();

          if (eval_node(node)) {
            if_block->true_found = true;
            output_enabled = true;
          } else {
            if_block->true_found = false;
            output_enabled = false;
          }

        } else {
          output_enabled = false;
        }
      }

      continue;
    }

    // ifdef directive
    if (strncmp(p, "ifdef", 5) == 0 && is_space(*(p+5))) {
      p += 5;
      if_block = new_if_block(if_block);

      if (output_enabled) {
        if_block->active = true;

        if (map_get(macros, next_ident(p))) {
          if_block->true_found = true;
          output_enabled = true;
        } else {
          if_block->true_found = false;
          output_enabled = false;
        }

      } else {
        if_block->active = false;
      }

      char *eol = strchr(p, '\n');
      if (!eol) break;
      p = eol + 1;

      continue;
    }

    // ifndef directive
    if (strncmp(p, "ifndef", 6) == 0 && is_space(*(p+6))) {
      p += 6;
      if_block = new_if_block(if_block);

      if (output_enabled) {
        if_block->active = true;

        if (!map_get(macros, next_ident(p))) {
          if_block->true_found = true;
          output_enabled = true;
        } else {
          if_block->true_found = false;
          output_enabled = false;
        }

      } else {
        if_block->active = false;
      }

      char *eol = strchr(p, '\n');
      if (!eol) break;
      p = eol + 1;

      continue;
    }

    // else directive
    if (strncmp(p, "else", 4) == 0 && isspace(*(p+4))) {
      if (!if_block) error_at(source, p, "not in if macro");
      if (if_block->active) {
        output_enabled = !if_block->true_found;
      }

      char *eol = strchr(p, '\n');
      if (!eol) break;
      p = eol + 1;

      continue;
    }

    // endif directive
    if (strncmp(p, "endif", 5) == 0 && isspace(*(p+5))) {
      if (!if_block) error_at(source, p, "not in if macro");
      if (if_block->active) {
        output_enabled = true;
      }

      if_block = if_block->parent;

      char *eol = strchr(p, '\n');
      if (!eol) break;
      p = eol + 1;

      continue;
    }

    // error directive
    if (strncmp(p, "error", 5) == 0 && is_space(*(p+5))) {
      p += 5;
      p = skip_spaces(p);

      char *eol = strchr(p, '\n');
      if (!eol) break;

      if (output_enabled) {
        error_at(source, p, substring(p, eol - p));
      }

      p = eol + 1;
      continue;
    }

    // warning directive
    if (strncmp(p, "warning", 7) == 0 && is_space(*(p+7))) {
      p += 7;
      p = skip_spaces(p);

      char *eol = strchr(p, '\n');
      if (!eol) break;

      if (output_enabled) {
        warning_at(source, p, substring(p, eol - p));
      }

      p = eol + 1;
      continue;
    }

    error_at(source, p, "failed to tokenize macro");
  }

  return head.next;
}

Token *replace_macro_params(Token *from, Token *to, Macro *macro) {
  from = copy_tokens(from);
  to = copy_tokens(to);

  Token *from_cur = from->next->next;  // MACRO -> ( -> PARAM1
  if (!from_cur) error("failed to replace macro params");

  Token head;
  head.next = to;
  Token *to_cur = &head;

  for (int i = 0; i < macro->params->len; i++) {
    char *name = macro->params->data[i];

    // Move from_cur to the next param
    Token *param_start = from_cur;
    int depth = 0;

    if (strcmp(from_cur->str, "(") == 0) depth++;
    while (from_cur->next) {
      if (strcmp(from_cur->next->str, "(") == 0) depth++;
      else if (depth != 0 && strcmp(from_cur->next->str, ")") == 0) depth--;
      else if (depth == 0 && (strcmp(from_cur->next->str, ")") == 0 || strcmp(from_cur->next->str, ",") == 0)) break;
      from_cur = from_cur->next;
    }
    Token *param_end = from_cur;
    from_cur = from_cur->next->next;  // ... -> , -> PARAM2 -> ...

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

      // Skip if macro is deleted
      if (!m) continue;

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
