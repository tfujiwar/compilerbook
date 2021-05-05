#include "mycc.h"

Token *token;
Node *code[100];
LVar *locals;
Map *functions;
Map *strings;
Scope *global;
Scope *scope;
SwitchScope *sw_scope;
BreakScope *br_scope;
Map *macros;
int labels = 0;


int main(int argc, char **argv) {
  Source *src;
  if (argc == 2) {
    src = new_source(NULL, argv[1]);
  } else {
    error("incorrect number of argument");
  }

  macros = new_map();
  locals = calloc(1, sizeof(LVar));
  locals->offset = 0;
  functions = new_map();
  map_put(functions, "printf", new_function("printf", type_int()));
  strings = new_map();
  global = new_scope(NULL);
  scope = global;
  sw_scope = new_switch_scope(NULL, NULL);
  br_scope = new_break_scope(NULL, NULL);

  char *user_input = preprocess(src);
  debug_macros();

  token = tokenize(user_input);
  debug_token(token);

  token = apply_macros(token, NULL);
  debug_token(token);

  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("\n");

  int i = 0;
  printf("  .data\n\n");
  for (int i = 0; code[i]; i++) {
    if (code[i]->kind == ND_DECLARE_GVAR) {
      code[i] = analyze(code[i], true);
      gen(code[i]);
    }
  }

  gen_string();

  printf("  .text\n\n");
  for (int i = 0; code[i]; i++) {
    if (code[i]->kind != ND_DECLARE_GVAR) {
      code[i] = analyze(code[i], true);
      debug_node(code[i], "", "");
      gen(code[i]);
    }
  }

  debug_scope(global);
  debug_functions();

  return 0;
}
