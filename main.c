#include "mycc.h"

// Define global variables
Token *token;
Node *code[1024];
LVar *locals;
Map *functions;
Map *strings;
Scope *scope;
SwitchScope *sw_scope;
BreakScope *br_scope;
Map *macros;
int labels = 0;

int main(int argc, char **argv) {
  // Read source code
  Source *src;
  if (argc == 2) {
    src = new_source(NULL, argv[1]);
  } else {
    src = new_source(NULL, "test/main.c");
  }

  // Initialize global variables
  locals = calloc(1, sizeof(LVar));
  functions = new_map();
  strings = new_map();
  sw_scope = new_switch_scope(NULL, NULL);
  br_scope = new_break_scope(NULL, NULL);
  macros = new_map();

  Scope *global = new_scope(NULL);
  scope = global;

  // Construct AST
  token = preprocess(src);
  program();

  // Generate Code
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
  debug_macros();
  debug_functions();

  return 0;
}
