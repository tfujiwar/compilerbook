#include "9cc.h"

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

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
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
