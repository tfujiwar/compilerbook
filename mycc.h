#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Vector Vector;

struct Vector {
  void **data;
  int capacity;
  int len;
};

Vector *new_vec();
void vec_push(Vector *vec, void *elem);

typedef struct Map Map;

struct Map {
  Vector *keys;
  Vector *vals;
};

Map *new_map();
void *vec_get(Vector *vec, int i);
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);

typedef enum {
  TK_RESERVED,
  TK_IDENT,
  TK_NUM,
  TK_RETURN,
  TK_IF,
  TK_ELSE,
  TK_FOR,
  TK_WHILE,
  TK_EOF,
  TK_VOID,
  TK_CHAR,
  TK_INT,
  TK_SIZEOF,
  TK_STRING,
  TK_STRUCT,
  TK_TYPEDEF,
} TokenKind;

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_MOD,
  ND_ASSIGN,
  ND_LVAR,
  ND_GVAR,
  ND_NUM,
  ND_EQ,
  ND_NE,
  ND_LE,
  ND_LT,
  ND_RETURN,
  ND_IF,
  ND_FOR,
  ND_WHILE,
  ND_BLOCK,
  ND_FUNC,
  ND_FUNC_NAME,
  ND_CALL,
  ND_ADDR,
  ND_DEREF,
  ND_SIZEOF,
  ND_DECLARE,
  ND_DECLARE_GVAR,
  ND_STRING,
  ND_LOGICAL_AND,
  ND_LOGICAL_OR,
  ND_BITWISE_AND,
  ND_BITWISE_OR,
  ND_BITWISE_XOR,
  ND_SHIFT_LEFT,
  ND_SHIFT_RIGHT,
  ND_BITWISE_NOT,
  ND_CONDITIONAL,
  ND_COMMA,
  ND_DOT,
  ND_ARROW,
} NodeKind;

typedef struct Member Member;

struct Member {
  char *name;
  struct Type *type;
  int offset;
};

typedef struct Struct Struct;

struct Struct {
  char *name;
  Map *member;
};

typedef struct Type Type;

struct Type {
  enum { VOID, CHAR, INT, PTR, ARRAY, STRUCT } ty;
  struct Type *ptr_to;
  size_t size;
  size_t array_size;
  Struct *strct;
};

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  char *at;
};

typedef struct LVar LVar;

struct LVar {
  LVar *next;
  char *name;
  int offset;
  Type *type;
  bool is_global;
};

typedef struct Function Function;

struct Function {
  char *name;
  Vector *args;
  Type *return_type;
};

typedef struct Scope Scope;

struct Scope {
  Scope *parent;
  Map *vars;
  Map *types;
};

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  Node *init;
  Node *cond;
  Node *inc;
  Node *body;
  Node *els;
  Node *child;
  Vector *children;
  Node *next;
  int val;
  char *name;
  Type *type;
  LVar *lvar;
  Function *func;
  Scope *scope;
};

char *read_file(char *path);
void debug(char *fmt, ...);
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void debug_token(Token *tok);
void debug_node(Node *node, char *pre1, char *pre2);
void debug_type(Type *type);
char *substring(char *str, int len);

bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
bool is_ident_char(char c);

Token *tokenize();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Scope *new_scope(Scope *parent);
Function *new_function(char *name, Type *type);
Struct *new_struct(char *name);
Member *new_member(char *name, Type *type, int offset);
Type *type();
Type *find_type(char *name);

void program();
Node *function();
Node *stmt();
Node *expr();
Node *assign();
Node *conditional();
Node *logical_or();
Node *logical_and();
Node *bitwise_or();
Node *bitwise_xor();
Node *bitwise_and();
Node *equality();
Node *relational();
Node *shift();
Node *add();
Node *mul();
Node *unary_left();
Node *unary_right();
Node *primary();

Type *type_int();
Node *analyze(Node *node, bool cast_array);

void gen(Node *node);
void gen_lval(Node *node);
void gen_string();

extern char *filename;
extern char *user_input;
extern Token *token;
extern Node *code[];
extern LVar *locals;
extern int labels;
extern Map *functions;
extern Map *strings;
extern Scope *global;
extern Scope *scope;
