#include "mycc.h"

char *read_file(char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp)
    error("cannot open %s: %s", path, strerror(errno));

  if (fseek(fp, 0, SEEK_END) == -1)
    error("%s: fseek: %s", path, strerror(errno));

  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1)
    error("%s: fseek: %s", path, strerror(errno));

  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  if (size == 0 || buf[size - 1] != '\n')
    buf[size++] = '\n';
  buf[size++] = '\0';
  fclose(fp);
  return buf;
}

void debug(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void print_error_cursor(Source *src, char *at) {
  char *begin = at;
  while (src->head < begin && begin[-1] != '\n')
    begin--;

  char *end = at;
  while (*end != '\0' && *end != '\n')
    end++;

  int line = 1;
  for (char *p = src->head; p < begin; p++)
    if (*p == '\n') line++;

  int indent = fprintf(stderr, "%s:%d: ", src->filename, line);
  fprintf(stderr, "%.*s\n", (int)(end - begin), begin);

  int pos = at - begin + indent;
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
}

void warning_at(Source *src, char* at, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  print_error_cursor(src, at);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");

  va_end(ap);
}

void error_at(Source *src, char* at, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  print_error_cursor(src, at);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");

  va_end(ap);
  exit(1);
}

void error_at_token(Token *token, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  print_error_cursor(token->src, token->at);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");

  va_end(ap);
  exit(1);
}

Vector *new_vec() {
  Vector *vec = malloc(sizeof(Vector));
  vec->len = 0;
  vec->capacity = 16;
  vec->data = malloc(sizeof(void *) * 16);
  return vec;
}

void *vec_get(Vector *vec, int i) {
  return vec->data[i];
}

void vec_push(Vector *vec, void *elem) {
  if (vec->len == vec->capacity) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

Map *new_map() {
  Map *map = malloc(sizeof(Map));
  map->keys = new_vec();
  map->vals = new_vec();
  return map;
}

void map_put(Map *map, char *key, void *val) {
  for (int i = 0; i < map->keys->len; i++) {
    if (strcmp(map->keys->data[i], key) == 0) {
      map->vals->data[i] = val;
      return;
    }
  }
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void *map_get(Map *map, char *key) {
  for (int i = 0; i < map->keys->len; i++) {
    if (strcmp(map->keys->data[i], key) == 0) {
      return map->vals->data[i];
    }
  }
  return NULL;
}

void map_delete(Map *map, char *key) {
  for (int i = 0; i < map->keys->len; i++) {
    if (strcmp(map->keys->data[i], key) == 0) {
      map->vals->data[i] = NULL;
      return;
    }
  }
}

char *substring(char *str, int len) {
  char *sub = malloc(sizeof(char) * (len + 1));
  strncpy(sub, str, len);
  sub[len] = '\x0';
  return sub;
}

char *convert_escaped_str(char *str) {
  char *converted = malloc(sizeof(char) * (strlen(str) + 1));
  int j = 0;
  for (int i = 0; i < strlen(str); i++) {
    if (str[i] == '\\') {
      i++;
      if (str[i] == 'a')  converted[j] = '\a';
      if (str[i] == 'b')  converted[j] = '\b';
      if (str[i] == 'f')  converted[j] = '\f';
      if (str[i] == 'n')  converted[j] = '\n';
      if (str[i] == 'r')  converted[j] = '\r';
      if (str[i] == 't')  converted[j] = '\t';
      if (str[i] == 'v')  converted[j] = '\v';
      if (str[i] == '\\') converted[j] = '\\';
      if (str[i] == '\?') converted[j] = '\?';
      if (str[i] == '\'') converted[j] = '\'';
      if (str[i] == '\"') converted[j] = '\"';
      if (str[i] == '\0') converted[j] = '\0';
    } else {
      converted[i] = str[j];
    }
    j++;
  }
  converted[j] = '\0';
  return converted;
}

void debug_token(Token *token) {
  Token *tok = token;
  while (tok) {
    switch (tok->kind) {
    case TK_RESERVED:
      fprintf(stderr, "%s ", tok->str);
      if (tok->str[0] == ';') fprintf(stderr, "\n");
      break;
    case TK_IDENT:
      fprintf(stderr, "IDENT(%s) ", tok->str);
      break;
    case TK_NUM:
      fprintf(stderr, "NUM(%d) ", tok->val);
      break;
    case TK_RETURN:
      fprintf(stderr, "RETURN ");
      break;
    case TK_IF:
      fprintf(stderr, "IF ");
      break;
    case TK_ELSE:
      fprintf(stderr, "ELSE ");
      break;
    case TK_FOR:
      fprintf(stderr, "FOR ");
      break;
    case TK_WHILE:
      fprintf(stderr, "WHILE ");
      break;
    case TK_DO:
      fprintf(stderr, "DO ");
      break;
    case TK_SWITCH:
      fprintf(stderr, "SWITCH ");
      break;
    case TK_CASE:
      fprintf(stderr, "CASE ");
      break;
    case TK_BREAK:
      fprintf(stderr, "BREAK ");
      break;
    case TK_DEFAULT:
      fprintf(stderr, "DEFAULT ");
      break;
    case TK_EOF:
      fprintf(stderr, "EOF\n");
      break;
    case TK_VOID:
      fprintf(stderr, "VOID ");
      break;
    case TK_INT:
      fprintf(stderr, "INT ");
      break;
    case TK_CHAR:
      fprintf(stderr, "CHAR ");
      break;
    case TK_SIZEOF:
      fprintf(stderr, "SIZEOF ");
      break;
    case TK_DEFINED:
      fprintf(stderr, "DEFINED ");
      break;
    case TK_STRING:
      fprintf(stderr, "\"%s\" ", tok->str);
      break;
    case TK_STRUCT:
      fprintf(stderr, "STRUCT ");
      break;
    case TK_TYPEDEF:
      fprintf(stderr, "TYPEDEF ");
      break;
    case TK_CONCAT:
      fprintf(stderr, "## ");
      break;
    case TK_STRINGIFY:
      fprintf(stderr, "# ");
      break;
    case TK_ELLIPSIS:
      fprintf(stderr, "... ");
      break;
    case TK_EXTERN:
      fprintf(stderr, "EXTERN ");
      break;
    }
    tok = tok->next;
  }
}

void debug_node(Node *node, char *pre1, char *pre2) {
  Node *cur;
  char p11[255], p21[255], p31[255], p41[255], p12[255], p22[255], p32[255], p42[255];
  char *label;
  char *type;

  if (!node) {
    fprintf(stderr, "%sNULL\n", pre1);
    return;
  }

  if (node->type) {
    switch (node->type->ty) {
    case CHAR:
      type = "CHR";
      break;
    case INT:
      type = "INT";
      break;
    case PTR:
      type = "PTR";
      break;
    case ARRAY:
      type = "ARR";
      break;
    case STRUCT:
      type = "STR";
      break;
    }
  } else {
    type = "NUL";
  }

  switch (node->kind) {
  case ND_LVAR:
    fprintf(stderr, "%s%s(%s)\n", pre1, node->lvar->name, type);
    return;

  case ND_GVAR:
    fprintf(stderr, "%s%s(%s)\n", pre1, node->lvar->name, type);
    return;

  case ND_NUM:
    fprintf(stderr, "%s%d(%s)\n", pre1, node->val, type);
    return;

  case ND_DECLARE:
    if (!node->child) {
      fprintf(stderr, "%sDECL\n", pre1);
      return;
    }
    cur = node->child;
    while (cur) {
      if (cur == node->child && !cur->next) {
        sprintf(p11, "%sDECL ----- ", pre1);
        sprintf(p12, "%s           ", pre2);
      } else if (cur == node->child) {
        sprintf(p11, "%sDECL ----+ ", pre1);
        sprintf(p12, "%s         | ", pre2);
      } else if (cur->next) {
        sprintf(p11, "%s         L ", pre2);
        sprintf(p12, "%s         | ", pre2);
      } else {
        sprintf(p11, "%s         L ", pre2);
        sprintf(p12, "%s           ", pre2);
      }
      debug_node(cur, p11, p12);
      cur = cur->next;
    }
    return;

  case ND_DECLARE_GVAR:
    fprintf(stderr, "%sDECLG(%s)\n", pre1, node->lvar->name);
    return;

  case ND_STRING:
    fprintf(stderr, "%sSTRING\n", pre1);
    return;

  case ND_DEFAULT:
    fprintf(stderr, "%sDEFAULT\n", pre1);
    return;

  case ND_BREAK:
    fprintf(stderr, "%sBREAK\n", pre1);
    return;

  case ND_FUNC_NAME:
    fprintf(stderr, "%sFUNC_NAME(%s)\n", pre1, node->func->name);
    return;

  case ND_CASE:
    sprintf(p11, "%sCASE ----- ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_RETURN:
    sprintf(p11, "%sRET ------ ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_FUNC:
    sprintf(p11, "%sFUNC ----- ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->body, p11, p12);
    return;

  case ND_ADDR:
    sprintf(p11, "%sADR(%s) - ", pre1, type);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_DEREF:
    sprintf(p11, "%sDER(%s) - ", pre1, type);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_SIZEOF:
    sprintf(p11, "%sSIZEOF --- ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_BITWISE_NOT:
    sprintf(p11, "%s~ -------- ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_IF:
    sprintf(p11, "%sIF ------+ ", pre1);
    sprintf(p12, "%s         | ", pre2);
    sprintf(p21, "%s         L ", pre2);
    sprintf(p22, "%s         | ", pre2);
    sprintf(p31, "%s         L ", pre2);
    sprintf(p32, "%s           ", pre2);
    debug_node(node->cond, p11, p12);
    debug_node(node->body, p21, p22);
    debug_node(node->els, p31, p32);
    return;

  case ND_FOR:
    sprintf(p11, "%sFOR -----+ ", pre1);
    sprintf(p12, "%s         | ", pre2);
    sprintf(p21, "%s         L ", pre2);
    sprintf(p22, "%s         | ", pre2);
    sprintf(p31, "%s         L ", pre2);
    sprintf(p32, "%s         | ", pre2);
    sprintf(p41, "%s         L ", pre2);
    sprintf(p42, "%s           ", pre2);
    debug_node(node->init, p11, p12);
    debug_node(node->cond, p21, p22);
    debug_node(node->inc, p31, p32);
    debug_node(node->body, p41, p42);
    return;

  case ND_WHILE:
  case ND_DO_WHILE:
    sprintf(p11, "%sWHILE ---+ ", pre1);
    sprintf(p12, "%s         | ", pre2);
    sprintf(p21, "%s         L ", pre2);
    sprintf(p22, "%s           ", pre2);
    debug_node(node->cond, p11, p12);
    debug_node(node->body, p21, p22);
    return;

  case ND_SWITCH:
    sprintf(p11, "%sSWITCH --+ ", pre1);
    sprintf(p12, "%s         | ", pre2);
    sprintf(p21, "%s         L ", pre2);
    sprintf(p22, "%s           ", pre2);
    debug_node(node->cond, p11, p12);
    debug_node(node->body, p21, p22);
    return;

  case ND_CONDITIONAL:
    sprintf(p11, "%sCOND ----+ ", pre1);
    sprintf(p12, "%s         | ", pre2);
    sprintf(p21, "%s         L ", pre2);
    sprintf(p22, "%s         | ", pre2);
    sprintf(p31, "%s         L ", pre2);
    sprintf(p32, "%s           ", pre2);
    debug_node(node->cond, p11, p12);
    debug_node(node->body, p21, p22);
    debug_node(node->els, p31, p32);
    return;

  case ND_BLOCK:
    for (int i = 0; i < node->children->len; i++) {
      if (node->children->len == 1) {
        sprintf(p11, "%sBLOCK ---- ", pre1);
        sprintf(p12, "%s           ", pre2);
      } else if (i == 0) {
        sprintf(p11, "%sBLOCK ---+ ", pre1);
        sprintf(p12, "%s         | ", pre2);
      } else if (i != node->children->len - 1) {
        sprintf(p11, "%s         L ", pre2);
        sprintf(p12, "%s         | ", pre2);
      } else {
        sprintf(p11, "%s         L ", pre2);
        sprintf(p12, "%s           ", pre2);
      }
      debug_node(vec_get(node->children, i), p11, p12);
    }
    return;

  case ND_CALL:
    for (int i = 0; i < node->children->len; i++) {
      if (node->children->len == 1) {
        sprintf(p11, "%sCAL(%s) - ", pre1, type);
        sprintf(p12, "%s           ", pre2);
      } else if (i == 0) {
        sprintf(p11, "%sCAL(%s) + ", pre1, type);
        sprintf(p12, "%s         | ", pre2);
      } else if (i != node->children->len - 1) {
        sprintf(p11, "%s         L ", pre2);
        sprintf(p12, "%s         | ", pre2);
      } else {
        sprintf(p11, "%s         L ", pre2);
        sprintf(p12, "%s           ", pre2);
      }
      debug_node(vec_get(node->children, i), p11, p12);
    }
    return;

  case ND_DOT:
    fprintf(stderr, "%sDOT -----+ %s\n", pre1, node->name);
    sprintf(p21, "%s         L ", pre2);
    sprintf(p22, "%s           ", pre2);
    debug_node(node->lhs, p21, p22);
    return;

  case ND_ARROW:
    fprintf(stderr, "%sARROW ---+ %s\n", pre1, node->name);
    sprintf(p21, "%s         L ", pre2);
    sprintf(p22, "%s           ", pre2);
    debug_node(node->lhs, p21, p22);
    return;

  case ND_ADD: label = "ADD"; break;
  case ND_SUB: label = "SUB"; break;
  case ND_MUL: label = "MUL"; break;
  case ND_DIV: label = "DIV"; break;
  case ND_MOD: label = "MOD"; break;
  case ND_ASSIGN: label = "ASS"; break;
  case ND_EQ: label = "EQ "; break;
  case ND_NE: label = "NE "; break;
  case ND_LE: label = "LE "; break;
  case ND_LT: label = "LT "; break;
  case ND_LOGICAL_AND: label = "&& "; break;
  case ND_LOGICAL_OR: label = "|| "; break;
  case ND_BITWISE_AND: label = "&  "; break;
  case ND_BITWISE_OR: label = "|  "; break;
  case ND_BITWISE_XOR: label = "^  "; break;
  case ND_SHIFT_LEFT: label = "<< "; break;
  case ND_SHIFT_RIGHT: label = ">> "; break;
  case ND_COMMA: label = ",  "; break;

  default:
    error("failed to visualize node: %d", node->kind);
  }

  sprintf(p11, "%s%s(%s) + ", pre1, label, type);
  sprintf(p12, "%s         | ", pre2);
  sprintf(p21, "%s         L ", pre2);
  sprintf(p22, "%s           ", pre2);
  debug_node(node->lhs, p11, p12);
  debug_node(node->rhs, p21, p22);
  return;
}

void debug_type(Type *type) {
  if (type->ty == VOID) {
    fprintf(stderr, "VOID\n");

  } else if (type->ty == INT) {
    fprintf(stderr, "INT\n");

  } else if (type->ty == CHAR) {
    fprintf(stderr, "CHAR\n");

  } else if (type->ty == PTR) {
    fprintf(stderr, "PTR -> ");
    debug_type(type->ptr_to);

  } else if (type->ty == ARRAY) {
    fprintf(stderr, "ARRAY[%d] -> ", type->array_size);
    debug_type(type->ptr_to);

  } else if (type->ty == STRUCT) {
    fprintf(stderr, "%s\n", type->strct->name);
  }
}

void debug_scope(Scope *scope) {
  debug("debug scopes:");
  Scope *sc = scope;
  while (sc) {
    debug("scope:");

    debug("type:");
    for (int i = 0; i < sc->types->keys->len; i++) {
      fprintf(stderr, "- %s: ", sc->types->keys->data[i]);
      debug_type(sc->types->vals->data[i]);
    }

    debug("struct:");
    for (int i = 0; i < sc->structs->keys->len; i++) {
      fprintf(stderr, "- %s: ", sc->structs->keys->data[i]);
      debug_type(sc->structs->vals->data[i]);
    }

    debug("enum:");
    for (int i = 0; i < sc->enums->keys->len; i++) {
      fprintf(stderr, "- %s\n", sc->enums->keys->data[i]);
    }

    debug("variable:");
    for (int i = 0; i < sc->vars->keys->len; i++) {
      fprintf(stderr, "- %s: ", sc->vars->keys->data[i]);
      debug_type(((LVar *)(sc->vars->vals->data[i]))->type);
    }

    debug("");
    sc = sc->parent;
  }
}

void debug_functions() {
  for (int i = 0; i < functions->keys->len; i++) {
    Function *func = functions->vals->data[i];
    fprintf(stderr, "%s: ", functions->keys->data[i]);
    debug_type(func->return_type);

    for (int j = 0; j < func->arg_types->len; j++) {
      Type *type = func->arg_types->data[j];
      Node *arg = func->args->data[j];
      if (func->args->len)
        fprintf(stderr, "- %s %d: ", arg->lvar->name, arg->lvar->offset);
      else
        fprintf(stderr, "- :");
      debug_type(type);
    }

    debug("");
  }
}

void debug_macros() {
  for (int i = 0; i < macros->keys->len; i++) {
    Macro *macro = macros->vals->data[i];
    if (!macro) continue;
    debug("%s:", macros->keys->data[i]);
    fprintf(stderr, "- from : ");
    debug_token(macro->from);
    debug("");
    fprintf(stderr, "- to   : ");
    debug_token(macro->to);
  }
}
