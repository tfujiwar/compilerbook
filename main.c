#include "9cc.h"

char *filename;
char *user_input;
Token *token;
Node *code[100];
LVar *locals;
Map *globals;
Map *functions;
int labels = 0;

int main(int argc, char **argv) {
  if (argc == 2) {
    filename = "input";
    user_input = argv[1];
  } else if (argc == 3 && strcmp(argv[1], "-f") == 0) {
    filename = argv[2];
    user_input = read_file(argv[2]);
  } else {
    error("incorrect number of argument");
  }

  token = tokenize();
  debug("");
  debug_token(token);

  locals = calloc(1, sizeof(LVar));
  locals->offset = 0;
  globals = new_map();
  functions = new_map();
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("\n");

  int i = 0;
  printf("  .bss\n\n");
  for (; code[i]->kind == ND_DECLARE_GVAR; i++) {
    code[i] = analyze(code[i]);
    gen(code[i]);
  }

  printf("  .text\n\n");
  for (; code[i]; i++) {
    debug_node(code[i], "", "");
    code[i] = analyze(code[i]);
    debug_node(code[i], "", "");
    gen(code[i]);
  }

  return 0;
}
