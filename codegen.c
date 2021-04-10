#include "9cc.h"

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("not a variable");
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");

}

void gen(Node *node) {
  int l, num;
  Node *cur;
  char name[64];

  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;

  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;

  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;

  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;

  case ND_IF:
    l = labels++;
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lelse%03d\n", l);
    gen(node->body);
    printf("  pop rax\n");
    printf("  jmp .Lend%03d\n", l);
    printf(".Lelse%03d:\n", l);
    if (node->els) {
      gen(node->els);
      printf("  pop rax\n");
    }
    printf(".Lend%03d:\n", l);
    printf("  push 0\n");  // dummy
    return;

  case ND_FOR:
    l = labels++;
    if (node->init){
      gen(node->init);
      printf("  pop rax\n");
    }
    printf(".Lbegin%03d:\n", l);
    if (node->cond) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%03d\n", l);
    }
    gen(node->body);
    printf("  pop rax\n");
    if (node->inc) {
      gen(node->inc);
      printf("  pop rax\n");
    }
    printf("  jmp .Lbegin%03d\n", l);
    printf(".Lend%03d:\n", l);
    printf("  push 0\n");  // dummy
    return;

  case ND_WHILE:
    l = labels++;
    printf(".Lbegin%03d:\n", l);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%03d\n", l);
    gen(node->body);
    printf("  pop rax\n");
    printf("  jmp .Lbegin%03d\n", l);
    printf(".Lend%03d:\n", l);
    printf("  push 0\n");  // dummy
    return;

  case ND_BLOCK:
    cur = node->child;
    while (cur) {
      gen(cur);
      printf("  pop rax\n");
      cur = cur->next;
    }
    printf("  push 0\n");  // dummy
    return;

  case ND_FUNC:
    strncpy(name, node->name, node->len);
    name[node->len] = '\x0';

    num = 0;
    cur = node->child;
    while (cur) {
      gen(cur);
      cur = cur->next;
      num++;
    }

    if (num >= 6) printf("  pop r9\n");
    if (num >= 5) printf("  pop r8\n");
    if (num >= 4) printf("  pop rcx\n");
    if (num >= 3) printf("  pop rdx\n");
    if (num >= 2) printf("  pop rsi\n");
    if (num >= 1) printf("  pop rdi\n");

    printf("  call %s\n", name);
    printf("  push rax\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;

  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;

  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;

  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;

  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;

  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;

  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;

  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}
