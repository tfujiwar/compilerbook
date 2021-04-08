#include "9cc.h"

Token *token;
char *user_input;
Node *code[100];
LVar *locals;

int main(int argc, char **argv) {
  if (argc != 2) {
    error("incorrect number of argument");
  }

  user_input = argv[1];
  token = tokenize();

  locals = calloc(1, sizeof(LVar));
  locals->offset = 0;
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  for (int i = 0; code[i]; i++)
    gen(code[i]);

  return 0;
}
