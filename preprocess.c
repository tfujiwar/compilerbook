#include "mycc.h"

typedef struct IfBlock IfBlock;

struct IfBlock {
  bool active;
  bool true_found;
  IfBlock* parent;
};

IfBlock *new_if_block(IfBlock *parent) {
  IfBlock *if_block = calloc(1, sizeof(IfBlock));
  if_block->parent = parent;
  return if_block;
}

char *next_ident(char *cur) {
  while (*cur == ' ') cur++;
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
      replace = new_token(TK_NUM, &dummy, "1", 1);
      replace->val = 1;
    } else {
      Token dummy;
      replace = new_token(TK_NUM, &dummy, "0", 1);
      replace->val = 0;
    }

    replace_tokens(begin, end, replace, replace);
    cur = replace;
  }
  return head.next;
}

char* preprocess(char *user_input) {
  char *output = calloc(strlen(user_input), sizeof(char));
  char *p = user_input;
  char *q = output;

  Token head;
  head.next = NULL;

  bool output_enabled = true;
  IfBlock *if_block = NULL;

  while (*p) {
    // Not a macro directive
    if (*p != '#') {
      char *eol = strchr(p, '\n');
      if (!eol) break;

      int n = eol - p + 1;
      if (output_enabled) {
        memcpy(q, p, n);
        q += n;
      }

      p += n;
      continue;
    }

    // Read # and spaces
    p++;
    while (*p == ' ') p++;

    // Define directive
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
      if (!eol) break;
      macro->to = tokenize(substring(p, eol - p));

      map_put(macros, macro->name ,macro);

      p = eol + 1;
      continue;
    }

    // if directive
    if (strncmp(p, "if", 2) == 0 && *(p+2) == ' ') {
      p += 2;
      if_block = new_if_block(if_block);

      char *eol = strchr(p, '\n');
      if (!eol) break;

      if (output_enabled) {
        if_block->active = true;

        token = tokenize(substring(p, eol - p + 1));
        token = replace_defined(token);
        token = apply_macros(token, NULL);
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

      p = eol + 1;
      continue;
    }

    // elif directive
    if (strncmp(p, "elif", 4) == 0 && *(p+4) == ' ') {
      p += 4;

      char *eol = strchr(p, '\n');
      if (!eol) break;

      if (if_block->active) {
        if (!if_block->true_found) {
          token = tokenize(substring(p, eol - p + 1));
          token = replace_defined(token);
          token = apply_macros(token, NULL);
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

      p = eol + 1;
      continue;
    }

    // ifdef directive
    if (strncmp(p, "ifdef", 5) == 0 && *(p+5) == ' ') {
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
    if (strncmp(p, "ifndef", 6) == 0 && *(p+6) == ' ') {
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
      p += 4;
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
      p += 5;

      if (if_block->active) {
        output_enabled = true;
      }

      if_block = if_block->parent;

      char *eol = strchr(p, '\n');
      if (!eol) break;
      p = eol + 1;

      continue;
    }

    error_at(p, "failed to tokenize macro");
  }
  return output;
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
