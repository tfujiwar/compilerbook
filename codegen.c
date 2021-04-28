#include "mycc.h"

char *word_ptr(Node *node) {
  switch (node->type->size) {
    case 1:
      return "BYTE PTR";
    case 2:
      return "WORD PTR";
    case 4:
      return "DWORD PTR";
    case 8:
      return "QWORD PTR";
  }
}

char *reg_a(Node *node) {
  switch (node->type->size) {
    case 1:
      return "al";
    case 2:
      return "ax";
    case 4:
      return "eax";
    case 8:
      return "rax";
  }
}

char *reg_b(Node *node) {
  switch (node->type->size) {
    case 1:
      return "bl";
    case 2:
      return "bx";
    case 4:
      return "ebx";
    case 8:
      return "rbx";
  }
}

char *reg_c(Node *node) {
  switch (node->type->size) {
    case 1:
      return "cl";
    case 2:
      return "cx";
    case 4:
      return "ecx";
    case 8:
      return "rcx";
  }
}

char *reg_d(Node *node) {
  switch (node->type->size) {
    case 1:
      return "dl";
    case 2:
      return "dx";
    case 4:
      return "edx";
    case 8:
      return "rdx";
  }
}

char *reg_di(Node *node) {
  switch (node->type->size) {
    case 1:
      return "dil";
    case 2:
      return "di";
    case 4:
      return "edi";
    case 8:
      return "rdi";
  }
}

char *reg_si(Node *node) {
  switch (node->type->size) {
    case 1:
      return "sil";
    case 2:
      return "si";
    case 4:
      return "esi";
    case 8:
      return "rsi";
  }
}

char *reg_8(Node *node) {
  switch (node->type->size) {
    case 1:
      return "r8b";
    case 2:
      return "r8w";
    case 4:
      return "r8d";
    case 8:
      return "r8";
  }
}

char *reg_9(Node *node) {
  switch (node->type->size) {
    case 1:
      return "r9b";
    case 2:
      return "r9w";
    case 4:
      return "r9d";
    case 8:
      return "r9";
  }
}

char *byte(Type *type) {
  switch (type->size) {
    case 1:
      return ".byte";
    case 2:
      return ".word";
    case 4:
      return ".long";
    case 8:
      return ".quad";
  }
}


void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("not a variable");
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->lvar->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  int l, num;
  Node *cur;
  char name[64];
  int bytes;

  switch (node->kind) {
  case ND_NUM:
    printf("  # ND_NUM\n");
    printf("  push %d\n", node->val);
    return;

  case ND_FUNC_NAME:
    printf("  # ND_FUNC_NAME\n");
    printf("  push OFFSET FLAT:%s\n", node->func->name);
    return;

  case ND_LVAR:
    printf("  # ND_LVAR\n");
    gen_lval(node);
    printf("  pop rax\n");
    if (node->type->size < 4) printf("  movsx eax, %s [rax]\n", word_ptr(node));
    else                      printf("  mov %s, %s [rax]\n", reg_a(node), word_ptr(node));
    printf("  push rax\n");
    return;

  case ND_GVAR:
    printf("  # ND_GVAR\n");
    printf("  lea rax, %s [%s]\n", word_ptr(node), node->lvar->name);
    if (node->type->size < 4) printf("  movsx eax, %s [rax]\n", word_ptr(node));
    else                      printf("  mov %s, %s [rax]\n", reg_a(node), word_ptr(node));
    printf("  push rax\n");
    return;

  case ND_ASSIGN:
    printf("  # ND_ASSIGN\n");
    if (node->lhs->kind == ND_GVAR) {
      gen(node->rhs);
      printf("  pop rdi\n");
      printf("  lea rax, %s [%s]\n", word_ptr(node), node->lhs->lvar->name);
      printf("  mov %s [rax], %s\n", word_ptr(node), reg_di(node));
      printf("  push rdi\n");
      return;
    }
    if (node->lhs->kind == ND_DEREF) gen(node->lhs->lhs);
    else gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov %s [rax], %s\n", word_ptr(node), reg_di(node));
    printf("  push rdi\n");
    return;

  case ND_RETURN:
    printf("  # ND_RETURN\n");
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;

  case ND_IF:
    printf("  # ND_IF\n");
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
    printf("  # ND_FOR\n");
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
    printf("  # ND_WHILE\n");
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
    printf("  # ND_BLOCK\n");
    cur = node->child;
    while (cur) {
      gen(cur);
      printf("  pop rax\n");
      printf("\n");
      cur = cur->next;
    }
    printf("  push 0\n");  // dummy
    printf("\n");
    return;

  case ND_FUNC:
    printf("# ND_FUNC\n");
    printf("%s:\n", node->func->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", (node->val + 15) / 16 * 16);

    for (int i = 0; i < node->func->args->len; i++) {
      Node *nd = node->func->args->data[i];
      gen_lval(nd);
      printf("  pop rax\n");
      if (num == 0) printf("  mov [rax], %s\n", reg_di(nd));
      if (num == 1) printf("  mov [rax], %s\n", reg_si(nd));
      if (num == 2) printf("  mov [rax], %s\n", reg_d(nd));
      if (num == 3) printf("  mov [rax], %s\n", reg_c(nd));
      if (num == 4) printf("  mov [rax], %s\n", reg_8(nd));
      if (num == 5) printf("  mov [rax], %s\n", reg_9(nd));
    }

    gen(node->body);
    return;

  case ND_CALL:
    printf("  # ND_CALL\n");
    gen(node->lhs);

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

    printf("  pop rax\n");
    printf("  call rax\n");
    printf("  push rax\n");
    return;

  case ND_ADDR:
    printf("  # ND_ADDR\n");
    if (node->lhs->kind == ND_GVAR) {
      printf("  lea rax, %s [%s]\n", word_ptr(node), node->lhs->lvar->name);
      printf("  push rax\n");
      return;
    }
    gen_lval(node->lhs);
    return;

  case ND_DEREF:
    printf("  # ND_DEREF\n");
    gen(node->lhs);
    printf("  pop rax\n");
    if (node->type->size < 4) printf("  movsx eax, %s [rax]\n", word_ptr(node));
    else                      printf("  mov %s, %s [rax]\n", reg_a(node), word_ptr(node));
    printf("  push rax\n");
    return;

  case ND_BITWISE_NOT:
    printf("  # ND_BITWISE_NOT\n");
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  not rax\n");
    printf("  push rax\n");
    return;

  case ND_DECLARE:
    cur = node->child;
    while (cur) {
      gen(cur);
      cur = cur->next;
      printf("  pop rax\n");
    }
    printf("  push 0\n");  // dummy
    return;

  case ND_DECLARE_GVAR:
    printf("# ND_DECLARE_GVAR\n");
    printf("%s:\n", node->lvar->name);

    bytes = 0;
    cur = node->rhs;
    while (cur) {
      if (cur->kind == ND_NUM) {
        if (node->lvar->type->ty == ARRAY) {
          printf("  %s %d\n", byte(node->lvar->type->ptr_to), cur->val);
          bytes += node->type->ptr_to->size;
        } else {
          printf("  %s %d\n", byte(node->lvar->type), cur->val);
          bytes += node->type->size;
        }
      } else {
        error("cannot initialize global variable: %s", node->lvar->name);
      }
      cur = cur->next;
    }

    if (node->type->size > bytes) {
      printf("  .zero %d\n", node->type->size - bytes);
    }

    printf("\n");
    return;

  case ND_STRING:
    printf("# ND_STRING\n");
    printf("  mov rax, OFFSET FLAT:.Lstr%03d\n", node->val);
    printf("  push rax\n");
    return;

  case ND_LOGICAL_AND:
    l = labels++;
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lfalse%03d\n", l);
    gen(node->rhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lfalse%03d\n", l);
    printf("  push 1\n");
    printf("  jmp .Lend%03d\n", l);
    printf(".Lfalse%03d:\n", l);
    printf("  push 0\n");
    printf(".Lend%03d:\n", l);
    return;

  case ND_LOGICAL_OR:
    l = labels++;
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  jne .Ltrue%03d\n", l);
    gen(node->rhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  jne .Ltrue%03d\n", l);
    printf("  push 0\n");
    printf("  jmp .Lend%03d\n", l);
    printf(".Ltrue%03d:\n", l);
    printf("  push 1\n");
    printf(".Lend%03d:\n", l);
    return;

  case ND_CONDITIONAL:
    l = labels++;
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lelse%03d\n", l);
    gen(node->body);
    printf("  jmp .Lend%03d\n", l);
    printf(".Lelse%03d:\n", l);
    gen(node->els);
    printf(".Lend%03d:\n", l);
    return;

  case ND_COMMA:
    gen(node->lhs);
    printf("  pop rax\n");
    gen(node->rhs);
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

  case ND_MOD:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    printf("  mov rax, rdx\n");
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

  case ND_BITWISE_AND:
    printf("  and rax, rdi\n");
    break;

  case ND_BITWISE_OR:
    printf("  or rax, rdi\n");
    break;

  case ND_BITWISE_XOR:
    printf("  xor rax, rdi\n");
    break;

  case ND_SHIFT_LEFT:
    printf("  mov cl, dil\n");
    printf("  shl rax, cl\n");
    break;

  case ND_SHIFT_RIGHT:
    printf("  mov cl, dil\n");
    printf("  shr rax, cl\n");
    break;
  }

  printf("  push rax\n");
  printf("\n");
}

void gen_string() {
  for (int i = 0; i < strings->keys->len; i++) {
    char *str = vec_get(strings->keys, i);
    printf(".Lstr%03d:\n", i);
    printf("  .string \"%s\"\n", str);
  }
}
