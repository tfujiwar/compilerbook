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

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  char *begin = loc;
  while (user_input < begin && begin[-1] != '\n')
    begin--;

  char *end = loc;
  while (*end != '\0' && *end != '\n')
    end++;

  int line = 1;
  for (char *p = user_input; p < begin; p++)
    if (*p == '\n') line++;

  int indent = fprintf(stderr, "%s:%d: ", filename, line);
  fprintf(stderr, "%.*s\n", (int)(end - begin), begin);

  int pos = loc - begin + indent;
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");

  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
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

char *substring(char *str, int len) {
  char *sub = malloc(sizeof(char) * (len + 1));
  strncpy(sub, str, len);
  sub[len] = '\x0';
  return sub;
}

void debug_token(Token *token) {
  Token *tok = token;
  while (tok) {
    switch (tok->kind) {
    case TK_RESERVED:
      fprintf(stderr, "%s ", tok->str);
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
    case TK_EOF:
      fprintf(stderr, "EOF\n");
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
    case TK_STRING:
      fprintf(stderr, "STRING");
      break;
    case TK_STRUCT:
      fprintf(stderr, "STRUCT");
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
        sprintf(p11, "%sDECL ───── ", pre1);
        sprintf(p12, "%s           ", pre2);
      } else if (cur == node->child) {
        sprintf(p11, "%sDECL ────┬ ", pre1);
        sprintf(p12, "%s         │ ", pre2);
      } else if (cur->next) {
        sprintf(p11, "%s         ├ ", pre2);
        sprintf(p12, "%s         │ ", pre2);
      } else {
        sprintf(p11, "%s         └ ", pre2);
        sprintf(p12, "%s           ", pre2);
      }
      debug_node(cur, p11, p12);
      cur = cur->next;
    }
    return;

  case ND_DECLARE_GVAR:
    fprintf(stderr, "%sDECL\n", pre1);
    return;

  case ND_STRING:
    fprintf(stderr, "%sSTRING\n", pre1);
    return;

  case ND_RETURN:
    sprintf(p11, "%sRET ────── ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_FUNC:
    sprintf(p11, "%sFUNC ───── ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->body, p11, p12);
    return;

  case ND_ADDR:
    sprintf(p11, "%sADR(%s) ─ ", pre1, type);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_DEREF:
    sprintf(p11, "%sDER(%s) ─ ", pre1, type);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_SIZEOF:
    sprintf(p11, "%sSIZEOF ─── ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_BITWISE_NOT:
    sprintf(p11, "%s~ ──────── ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_IF:
    sprintf(p11, "%sIF ──────┬ ", pre1);
    sprintf(p12, "%s         │ ", pre2);
    sprintf(p21, "%s         ├ ", pre2);
    sprintf(p22, "%s         │ ", pre2);
    sprintf(p31, "%s         └ ", pre2);
    sprintf(p32, "%s           ", pre2);
    debug_node(node->cond, p11, p12);
    debug_node(node->body, p21, p22);
    debug_node(node->els, p31, p32);
    return;

  case ND_FOR:
    sprintf(p11, "%sFOR ─────┬ ", pre1);
    sprintf(p12, "%s         │ ", pre2);
    sprintf(p21, "%s         ├ ", pre2);
    sprintf(p22, "%s         │ ", pre2);
    sprintf(p31, "%s         ├ ", pre2);
    sprintf(p32, "%s         │ ", pre2);
    sprintf(p41, "%s         └ ", pre2);
    sprintf(p42, "%s           ", pre2);
    debug_node(node->init, p11, p12);
    debug_node(node->cond, p21, p22);
    debug_node(node->inc, p31, p32);
    debug_node(node->body, p41, p42);
    return;

  case ND_WHILE:
    sprintf(p11, "%sWHILE ───┬ ", pre1);
    sprintf(p12, "%s         │ ", pre2);
    sprintf(p21, "%s         └ ", pre2);
    sprintf(p22, "%s           ", pre2);
    debug_node(node->cond, p11, p12);
    debug_node(node->body, p21, p22);
    return;

  case ND_CONDITIONAL:
    sprintf(p11, "%sCOND ────┬ ", pre1);
    sprintf(p12, "%s         │ ", pre2);
    sprintf(p21, "%s         ├ ", pre2);
    sprintf(p22, "%s         │ ", pre2);
    sprintf(p31, "%s         └ ", pre2);
    sprintf(p32, "%s           ", pre2);
    debug_node(node->cond, p11, p12);
    debug_node(node->body, p21, p22);
    debug_node(node->els, p31, p32);
    return;

  case ND_BLOCK:
    for (int i = 0; i < node->children->len; i++) {
      if (node->children->len == 1) {
        sprintf(p11, "%sBLOCK ──── ", pre1);
        sprintf(p12, "%s           ", pre2);
      } else if (i == 0) {
        sprintf(p11, "%sBLOCK ───┬ ", pre1);
        sprintf(p12, "%s         │ ", pre2);
      } else if (i != node->children->len - 1) {
        sprintf(p11, "%s         ├ ", pre2);
        sprintf(p12, "%s         │ ", pre2);
      } else {
        sprintf(p11, "%s         └ ", pre2);
        sprintf(p12, "%s           ", pre2);
      }
      debug_node(vec_get(node->children, i), p11, p12);
    }
    return;

  case ND_CALL:
    for (int i = 0; i < node->children->len; i++) {
      if (node->children->len == 1) {
        sprintf(p11, "%sCAL(%s) ─ ", pre1, type);
        sprintf(p12, "%s           ", pre2);
      } else if (i == 0) {
        sprintf(p11, "%sCAL(%s) ┬ ", pre1, type);
        sprintf(p12, "%s         │ ", pre2);
      } else if (i != node->children->len - 1) {
        sprintf(p11, "%s         ├ ", pre2);
        sprintf(p12, "%s         │ ", pre2);
      } else {
        sprintf(p11, "%s         └ ", pre2);
        sprintf(p12, "%s           ", pre2);
      }
      debug_node(vec_get(node->children, i), p11, p12);
    }
    return;

  case ND_DOT:
    fprintf(stderr, "%sDOT ─────┬ %s\n", pre1, node->name);
    sprintf(p21, "%s         └ ", pre2);
    sprintf(p22, "%s           ", pre2);
    debug_node(node->lhs, p21, p22);
    return;

  case ND_ARROW:
    fprintf(stderr, "%sARROW ───┬ %s\n", pre1, node->name);
    sprintf(p21, "%s         └ ", pre2);
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
  }

  sprintf(p11, "%s%s(%s) ┬ ", pre1, label, type);
  sprintf(p12, "%s         │ ", pre2);
  sprintf(p21, "%s         └ ", pre2);
  sprintf(p22, "%s           ", pre2);
  debug_node(node->lhs, p11, p12);
  debug_node(node->rhs, p21, p22);
  return;
}
