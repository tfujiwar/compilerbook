#include "/src/test/common.h"
#if 0
#include "nothing.h"
#endif

#define ONE 1
#define X_PLUS_Y (x+y)
#define X_PLUS_ONE (x+ONE)

#define X Y >= Z
#define Y Z * 5
#define Z (Y + 7)

#define FUNC(A,B,C) A+B+C+X_PLUS_Y

#define rep(i, n)\
        for (int i = 0; i < (n); i++)

// #define foo foo a bar b baz c
// #define bar foo 1
// #define baz bar 2
// % foo %  // foo a foo 1 b foo 1 2 c

// #define foo(a, b) foo a bar str(b)
// #define bar foo bar 1
// % foo(bar, (1, 2, 3)) %  // foo foo bar 1 foo bar 1 str((1, 2, 3))

// this is line comment

/*
this is block comment
*/

int gint_zero;
int garr_zero[5];
char gstr_zero[5];

int gint = 1;
int garr[5] = {1, 2, 3};
char gstr[5] = "ABC";

struct Struct {
  int i;
  int j;
  char k;
};

int function(int, char, struct Struct *);

int fii(int a, int b) {
  return a + b;
}

char fcc(char a, char b) {
  return a + b;
}

int fpp(int *a, int *b) {
  return *a + *b;
}

int sum(int *a, int len) {
  int ans = 0;
  for (int i = 0; i < len; i=i+1) {
    ans += a[i];
  }
  return ans;
}

int main() {
  { assert("0", 0, 0); }
  { assert("42", 42, 42); }
  { assert("'A'", 'A', 65); }

  { assert("5+20-4", 5+20-4, 21); }
  { assert("12 + 34 - 5 ", 12 + 34 - 5 , 41); }
  { assert("5+6*7", 5+6*7, 47); }
  { assert("5*(9-6)", 5*(9-6), 15); }
  { assert("(3+5)/2", (3+5)/2, 4); }
  { assert("-1+2", -1+2, 1); }
  { assert("-3*+5+20", -3*+5+20, 5); }
  { assert("10%3", 10%3, 1); }

  { assert("1==1", 1==1, 1); }
  { assert("1==0", 1==0, 0); }
  { assert("1!=1", 1!=1, 0); }
  { assert("1!=0", 1!=0, 1); }
  { assert("1<1", 1<1, 0); }
  { assert("1<2", 1<2, 1); }
  { assert("1>1", 1>1, 0); }
  { assert("2>1", 2>1, 1); }
  { assert("1<=0", 1<=0, 0); }
  { assert("1<=1", 1<=1, 1); }
  { assert("0>=1", 0>=1, 0); }
  { assert("1>=1", 1>=1, 1); }

  { assert("1 && 1", 1 && 1, 1); }
  { assert("1 && 0", 1 && 0, 0); }
  { assert("0 && 1", 0 && 1, 0); }
  { assert("0 && 0", 0 && 0, 0); }
  { int a=1; int b=1; (a=a-1) && (b=b-1); assert("b", b, 1); }
  { assert("1 || 1", 1 || 1, 1); }
  { assert("1 || 0", 1 || 0, 1); }
  { assert("0 || 1", 0 || 1, 1); }
  { assert("0 || 0", 0 || 0, 0); }
  { int a=0; int b=0; (a=a+1) || (b=b+1); assert("b", b, 0); }

  { assert("1 & 2", 1 & 2, 0); }
  { assert("1 | 2", 1 | 2, 3); }
  { assert("1 ^ 3", 1 ^ 3, 2); }
  { assert("1 | 2 | 4 | 8 | 16", 1 | 2 | 4 | 8 | 16, 31); }

  { assert("1 << 2", 1 << 2, 4); }
  { assert("4 >> 2", 4 >> 2, 1); }
  { assert("4 >> 1 >> 1 << 1 << 1", 4 >> 1 >> 1 << 1 << 1, 4); }

  { int a; char b; a=1; b=1; assert("a==b", a==b, 1); }
  { int a; char b; a=1; b=0; assert("a==b", a==b, 0); }
  { int a; char b; a=1; b=1; assert("a!=b", a!=b, 0); }
  { int a; char b; a=1; b=0; assert("a!=b", a!=b, 1); }
  { int a; char b; a=1; b=1; assert("a<b", a<b, 0); }
  { int a; char b; a=1; b=2; assert("a<b", a<b, 1); }
  { int a; char b; a=1; b=1; assert("a>b", a>b, 0); }
  { int a; char b; a=2; b=1; assert("a>b", a>b, 1); }
  { int a; char b; a=1; b=0; assert("a<=b", a<=b, 0); }
  { int a; char b; a=1; b=1; assert("a<=b", a<=b, 1); }
  { int a; char b; a=0; b=1; assert("a>=b", a>=b, 0); }
  { int a; char b; a=1; b=1; assert("a>=b", a>=b, 1); }

  { char a; int b; a=1; b=1; assert("a==b", a==b, 1); }
  { char a; int b; a=1; b=0; assert("a==b", a==b, 0); }
  { char a; int b; a=1; b=1; assert("a!=b", a!=b, 0); }
  { char a; int b; a=1; b=0; assert("a!=b", a!=b, 1); }
  { char a; int b; a=1; b=1; assert("a<b", a<b, 0); }
  { char a; int b; a=1; b=2; assert("a<b", a<b, 1); }
  { char a; int b; a=1; b=1; assert("a>b", a>b, 0); }
  { char a; int b; a=2; b=1; assert("a>b", a>b, 1); }
  { char a; int b; a=1; b=0; assert("a<=b", a<=b, 0); }
  { char a; int b; a=1; b=1; assert("a<=b", a<=b, 1); }
  { char a; int b; a=0; b=1; assert("a>=b", a>=b, 0); }
  { char a; int b; a=1; b=1; assert("a>=b", a>=b, 1); }

  { int a; a=1; assert("a", a, 1); }
  { int a; a=1+2; a=a*2; assert("a/3", a/3, 2); }
  { int returnx; returnx=1+2; assert("returnx", returnx, 3); }

  { int a=1; int b=0; if (a>0) b=1; assert("b", b, 1); }
  { int a=0; int b=0; if (a>0) b=1; assert("b", b, 0); }
  { int a=1; int b=0; if (a>0) b=1; else b=0; assert("b", b, 1); }
  { int a=0; int b=0; if (a>0) b=1; else b=0; assert("b", b, 0); }

  { int ans; int i; ans=0; for (i=1; i<=10; i=i+1) ans=ans+i; assert("ans", ans, 55); }
  { int a; a=0; while (a<10) a=a+1; assert("a", a, 10); }

  {
    int i = 1;
    int do_while_ans = 0;
    do { do_while_ans += i; } while (i++ < 9);
    assert("do_while_ans", do_while_ans, 45);
  }

  { int a; { a=1; a=a+1; }  assert("a", a, 2); }
  { int a; { a=1; {a=a+1; a=a+1; } a=a+1; }  assert("a", a, 4); }

  { assert("fii(1,2)", fii(1,2), 3); }
  { assert("fii(fii(1,2), fii(3,4))", fii(fii(1,2), fii(3,4)), 10); }
  { assert("fcc(fcc(1,2), fcc(3,4))", fcc(fcc(1,2), fcc(3,4)), 10); }
  { int a; int b; a=1; b=2; assert("fpp(&a,&b)", fpp(&a,&b), 3); }

  { int x; int *y; x=3; y=&x; assert("*y", *y, 3); }
  { int x; int *y; y=&x; *y=3; assert("x", x, 3); }
  { int x; int *y; int **z; y=&x; z=&y; **z=3; assert("x", x, 3); }

  { int x; assert("sizeof(x)", sizeof(x), 4); }
  { int x; assert("sizeof(x+1)", sizeof(x+1), 4); }
  { int x; assert("sizeof(1+x)", sizeof(1+x), 4); }
  { int *x; assert("sizeof(x)", sizeof(x), 8); }
  { int *x; assert("sizeof(x+1)", sizeof(x+1), 8); }
  { int *x; assert("sizeof(1+x)", sizeof(1+x), 8); }
  { int a[10]; assert("sizeof(a[0])", sizeof(a[0]), 4); }
  { int a[10]; assert("sizeof(&a)", sizeof(&a), 8); }
  { int a[10]; assert("sizeof(a+1)", sizeof(a+1), 8); }
  { int a[10]; assert("sizeof(a)", sizeof(a), 40); }

  { int a[2]; *a=1; *(a+1)=2; int *p; p=a; assert("*p+*(p+1)", *p+*(p+1), 3); }
  { int a[2]; *a=1; *(a+1)=2; int *p; p=a+1; assert("*p+*(p-1)", *p+*(p-1), 3); }
  { int a[2]; a[0]=1; a[1]=2; assert("a[0]+a[1]", a[0]+a[1], 3); }

  { int a[2]; *(a+1)=1; assert("3", 3, 3); }
  { int a[2]; a[1]=1; assert("3", 3, 3); }

  { char a; a=1; assert("a", a, 1); }
  { char a; char b; a=1; b=2; assert("a+b", a+b, 3); }
  { char a; char b; a=1; b=2; assert("b-a", b-a, 1); }
  { char a; int b; a=1; b=2; assert("a+b", a+b, 3); }
  { char a; int b; a=1; b=2; assert("b-a", b-a, 1); }
  { char a[2]; a[0]=1; a[1]=2; assert("a[0]+a[1]", a[0]+a[1], 3); }
  { char a; assert("sizeof(a)", sizeof(a), 1); }
  { char a[2]; assert("sizeof(a)", sizeof(a), 2); }
  { char a; assert("sizeof(&a)", sizeof(&a), 8); }
  { char a; a=1; int b; b=a; assert("b", b, 1); }
  { int a; a=1; char b; b=a; assert("b", b, 1); }
  { char a; char b; a=-1; b=2; assert("a+b", a+b, 1); }
  { char a; char b; a=128; b=-128; assert("a==b", a==b, 1); }
  { int a; int b; a=128; b=-128; assert("a!=b", a!=b, 1); }

  { char *s; s="ABC"; assert("s[0]", s[0], 65); }
  { char *s; s="ABC"; assert("s[1]", s[1], 66); }
  { char *s; s="ABC"; assert("s[2]", s[2], 67); }
  { char *s; s="ABC"; assert("s[3]", s[3], 0); }

  { int a; a=10; { int a; a=100; a=a+1; } a=a+1; assert("a", a, 11); }

  { int a=1; assert("a", a, 1); }
  { int a=1; int b=a*2; assert("b", b, 2); }
  { int a[]={1,2,3}; assert("a[0]", a[0], 1); }
  { int a[5]={1,2,3}; assert("a[0]", a[0], 1); }
  { int a[5]={1,2,3}; assert("a[1]", a[1], 2); }
  { int a[5]={1,2,3}; assert("a[2]", a[2], 3); }
  { int a[5]={1,2,3}; assert("a[3]", a[3], 0); }
  { int a[5]={1,2,3}; assert("a[4]", a[4], 0); }
  { char a[]="ABC"; assert("a[0]", a[0], 65); }
  { char a[]="ABC"; assert("a[1]", a[1], 66); }
  { char a[]="ABC"; assert("a[2]", a[2], 67); }
  { char a[]="ABC"; assert("a[3]", a[3], 0); }
  { char a[]="ABC"; assert("sizeof(a)", sizeof(a), 4); }
  { char a[5]="ABC"; assert("a[0]", a[0], 65); }
  { char a[5]="ABC"; assert("a[1]", a[1], 66); }
  { char a[5]="ABC"; assert("a[2]", a[2], 67); }
  { char a[5]="ABC"; assert("a[3]", a[3], 0); }
  { char a[5]="ABC"; assert("a[4]", a[4], 0); }
  { char a[5]="ABC"; assert("sizeof(a)", sizeof(a), 5); }

  assert("gint_zero", gint_zero, 0);
  assert("garr_zero[0]", garr_zero[0], 0);
  assert("gstr_zero[0]", gstr_zero[0], 0);

  assert("gint", gint, 1);
  assert("garr[0]", garr[0], 1);
  assert("garr[4]", garr[4], 0);
  assert("gstr[0]", gstr[0], 65);
  assert("gstr[4]", gstr[4], 0);

  assert("sizeof(gint)", sizeof(gint), 4);
  assert("sizeof(garr)", sizeof(garr), 20);
  assert("sizeof(gstr)", sizeof(gstr), 5);

  { int a=2; a+=1+1; assert("a", a, 4); }
  { int a=2; a-=1+1; assert("a", a, 0); }
  { int a=2; a*=1+1; assert("a", a, 4); }
  { int a=2; a/=1+1; assert("a", a, 1); }
  { int a=10; a%=3;  assert("a", a, 1); }
  { int a=1; a<<=2;  assert("a", a, 4); }
  { int a=4; a>>=2;  assert("a", a, 1); }
  { int a=1; a&=3;  assert("a", a, 1); }
  { int a=1; a^=3;  assert("a", a, 2); }
  { int a=1; a|=2;  assert("a", a, 3); }

  { int a; int b; a=b=1; assert("a+b", a+b, 2); }
  { int a=1; int b=10; a+=b+=1; assert("a", a, 12); }

  { int a=0; int b=++a; assert("a+b", a+b, 2); }
  { int a=1; int b=--a; assert("a+b", a+b, 0); }
  { int a=0; assert("!a", !a, 1); }
  { int a=1; assert("!a", !a, 0); }
  { int a=-1; assert("~a", ~a, 0); }

  { assert("1?2:3", 1?2:3, 2); }
  { assert("0?2:3", 0?2:3, 3); }
  { assert("1?1?2:3:4", 1?1?2:3:4, 2); }
  { assert("1?0?2:3:4", 1?0?2:3:4, 3); }
  { assert("0?2:0?4:5", 0?2:0?4:5, 5); }

  { int a=0; int b=(a=a+1, a=a+2, a); assert("a+b", a+b, 6); }

  { int a=1; int b=a++; assert("a+b", a+b, 3); }
  { int a=2; int b=a--; assert("a+b", a+b, 3); }

  {
    int a[] = {1, 2, 3, 4, 5};
    assert("sum(a, 3)", sum(a, 5), 15);
  }

  {
    struct Nest {
      struct Struct a;
    };

    struct Node {
      int value;
      struct Node *next;
    };

    struct Struct a;
    a.i = 1;
    a.j = 2;
    a.k = 3;
    assert("a.i", a.i, 1);
    assert("a.j", a.j, 2);
    assert("a.k", a.k, 3);
    assert("(&a)->i", (&a)->i, 1);
    assert("(&a)->j", (&a)->j, 2);
    assert("(&a)->k", (&a)->k, 3);
    assert("sizeof(a)", sizeof(a), 12);

    struct Struct b;
    b = a;
    assert("b.i", b.i, 1);
    assert("b.j", b.j, 2);
    assert("b.k", b.k, 3);

    // TODO
    // struct Struct c;
    // c = b = a;
    // assert("c.i", c.i, 1);
    // assert("c.j", c.j, 2);
    // assert("c.k", c.k, 3);

    struct Struct arr[4];
    arr[0].i = 1;
    arr[0].j = 2;
    arr[0].k = 3;
    assert("arr[0].i", arr[0].i, 1);
    assert("arr[0].j", arr[0].j, 2);
    assert("arr[0].k", arr[0].k, 3);
    assert("sizeof(a)", sizeof(arr), 48);

    struct Nest nest;
    nest.a.i = 1;
    nest.a.j = 2;
    nest.a.k = 3;
    assert("nest.a.i", nest.a.i, 1);
    assert("nest.a.j", nest.a.j, 2);
    assert("nest.a.k", nest.a.k, 3);
    assert("sizeof(nest)", sizeof(nest), 12);

    struct Node n1;
    struct Node n2;
    struct Node n3;
    n1.value = 1;
    n2.value = 2;
    n3.value = 3;
    n1.next = &n2;
    n2.next = &n3;
    assert("n1.next->next->value", n1.next->next->value, 3);
  }

  {
    typedef struct Struct MyStruct;
    MyStruct mystruct;
    mystruct.i = 1;
    assert("mystruct.i", mystruct.i, 1);

    typedef int MyInt;
    MyInt myint = 10;
    assert("myint", myint, 10);

    typedef MyInt MyInt2;
    MyInt2 myint2 = 20;
    assert("myint2", myint2, 20);
  }

  {
    struct { int i; } anonymous;
    anonymous.i = 10;
    assert("anonymous.i", anonymous.i, 10);

    struct Named { int i; } named1;
    struct Named named2;
    named1.i = 10;
    named2.i = 20;
    assert("named1.i", named1.i, 10);
    assert("named2.i", named2.i, 20);

    typedef struct { int i; } WITH_TYPEDEF;
    WITH_TYPEDEF with_typedef;
    with_typedef.i = 30;
    assert("with_typedef.i", with_typedef.i, 30);
  }

  {
    enum MyEnum1 { A1, B1, C1 } my_enum_1;
    assert("A1", A1, 0);
    assert("B1", B1, 1);
    assert("C1", C1, 2);

    enum MyEnum2 { A2, B2 = 10, C2 };
    enum MyEnum2 my_enum_2 = B2;
    assert("A2", A2, 0);
    assert("B2", B2, 10);
    assert("C2", C2, 11);
    assert("my_enum_2", my_enum_2, 10);

    enum { A3, B3, C3, } my_enum_3 = A3;
    assert("my_enum_3", my_enum_3, 0);

    typedef enum { A4, B5, C6 } MyEnum4;
    MyEnum4 my_enum_4;
    assert("sizeof(my_enum_4)", sizeof(my_enum_4), 4);
  }

  {
    struct Struct str;
    str.i = 3;
    str.j = 4;
    str.k = 5;
    assert("function(1, 2, *str)", function(1, 2, &str), 15);
  }

  {
    int i = 0;
    int case_ans = 0;
    switch (i) {
    case 1:
      case_ans += 1;
    case 2:
      case_ans += 2;
    default:
      case_ans += 4;
    }
    assert("case_ans", case_ans, 4);
  }

  {
    int i = 1;
    int case_ans = 0;
    switch (i) {
    case 1:
      case_ans += 1;
    case 2:
      case_ans += 2;
    default:
      case_ans += 4;
    }
    assert("case_ans", case_ans, 7);
  }

  {
    int i = 2;
    int case_ans = 0;
    switch (i) {
    case 1:
      case_ans += 1;
    case 2:
      case_ans += 2;
    default:
      case_ans += 4;
    }
    assert("case_ans", case_ans, 6);
  }

  {
    int i = 1;
    int case_ans = 0;
    switch (i) {
    case 1:
      case_ans += 1;
      break;
    case 2:
      case_ans += 2;
      break;
    default:
      case_ans += 4;
    }
    assert("case_ans", case_ans, 1);
  }

  {
    int i = 2;
    int case_ans = 0;
    switch (i) {
    case 1:
      case_ans += 1;
      break;
    case 2:
      case_ans += 2;
      break;
    default:
      case_ans += 4;
    }
    assert("case_ans", case_ans, 2);
  }

  {
    int i = 10;
    int j = 1;
    int case_ans = 0;
    switch (i) {
    case 10:
      switch (j) {
        case 1:
          case_ans += 1;
          break;
        case 2:
          case_ans += 2;
          break;
      }
      case_ans += 10;
      break;
    case 20:
      switch (j) {
        case 1:
          case_ans += 1;
          break;
        case 2:
          case_ans += 2;
          break;
      }
      case_ans += 20;
      break;
    }
    assert("case_ans", case_ans, 11);
  }

  {
    int break_ans = 0;
    for (int i = 1;; i++) {
      for (int j = 1;; j++) {
        break_ans++;
        if (j == 5) break;
      }
      if (i == 3) break;
    }
    assert("break_ans", break_ans, 15);
  }

  {
    int i = 0;
    int break_for_ans = 0;
    switch (i) {
      for (int j = 0; j < 5; j++) {
        case 0:
          break_for_ans++;
          break;
      }
      case 1:
        break_for_ans += 10;
    }
    assert("break_for_ans", break_for_ans, 11);
  }

  {
    int x = 1;
    int y = 2;
    assert("ONE", ONE, 1);
    assert("X_PLUS_Y", X_PLUS_Y, 3);
    assert("FUNC(1,2,3)", FUNC(1,2,3), 9);
  }

  {
    int ifdef = 0;
#ifdef ONE
    ifdef += 1;  // added
#else
    ifdef += 2;
#endif
    ifdef += 4;  // added
#ifdef NOTHING
    ifdef += 8;
#else
    ifdef += 16;  // added
#endif
#ifndef NOTHING
#  ifdef ONE
    ifdef += 32;  // added
#  else
    ifdef += 64;
#  endif
#else
#  ifdef ONE
    ifdef += 128;
#  else
    ifdef += 256;
#  endif
#endif
    ifdef += 512;  // added
    assert("ifdef", ifdef, 565);
  }

  {
    int if_macro = 0;
#if 1
    if_macro += 1;
#else
    if_macro += 2;
#endif
#if 0
    if_macro += 4;
#else
    if_macro += 8;
#endif
    assert("if_macro", if_macro, 9);
  }

  {
    int if_macro_calc = 0;
#if (2+3-1)*5/4%3 == 2
    if_macro_calc += 1;  // added
#endif
#if 0 == 1
    if_macro_calc += 2;
#endif
#if 0 != 1
    if_macro_calc += 4;  // added
#endif
#if 0 < 1
    if_macro_calc += 8;  // added
#endif
#if 0 > 1
    if_macro_calc += 16;
#endif
#if 0 <= 1
    if_macro_calc += 32;  // added
#endif
#if 0 >= 1
    if_macro_calc += 64;
#endif
#if (1 & 2) == 0
    if_macro_calc += 128;  // added
#endif
#if (1 | 2) == 3
    if_macro_calc += 256;  // added
#endif
#if (1 ^ 3) == 2
    if_macro_calc += 512;  // added
#endif
#if (1 << 2) == 4
    if_macro_calc += 1024;  // added
#endif
#if (4 >> 2) == 1
    if_macro_calc += 2048;  // added
#endif
#if 1 && 1
    if_macro_calc += 4096;  // added
#endif
#if 1 || 0
    if_macro_calc += 8192;  // added
#endif
#if !0
    if_macro_calc += 16384;  // added
#endif
#if 1 ? 1 : 0
    if_macro_calc += 32768;  // added
#endif
    assert("if_macro_calc", if_macro_calc, 65453);
  }

  {
    int if_defined = 0;
#if defined ONE
    if_defined += 1;
#endif
#if defined NOTHING
    if_defined += 2;
#endif
    assert("if_macro_calc", if_defined, 1);
  }

  {
    int if_elif_macro = 0;
#if defined ONE
    if_elif_macro += 1;
#elif !defined NOTHING
    if_elif_macro += 2;
#else
    if_elif_macro += 4;
#endif
    assert("if_elif_macro", if_elif_macro, 1);
  }

  {
    int if_elif_macro = 0;
#if !defined ONE
    if_elif_macro += 1;
#elif !defined NOTHING
    if_elif_macro += 2;
#else
    if_elif_macro += 4;
#endif
    assert("if_elif_macro", if_elif_macro, 2);
  }


  {
    int if_elif_macro = 0;
#if !defined ONE
    if_elif_macro += 1;
#elif defined NOTHING
    if_elif_macro += 2;
#else
    if_elif_macro += 4;
#endif
    assert("if_elif_macro", if_elif_macro, 4);
  }

  {
#undef UNDEF_TEST
    int undef = 0;
#define UNDEF_TEST
#ifdef UNDEF_TEST
    undef += 1;
#endif
#undef UNDEF_TEST
#ifdef UNDEF_TEST
    undef += 2;
#endif
    assert("undef", undef, 1);
  }

  {
#if 0
#error "this is an error example"
#endif
#warning "this is a warning example"
  }

  {
    int replace_undefined = 0;
#if REPLACE_UNDEFINED == 0
    replace_undefined += 1;
#endif
#define REPLACE_UNDEFINED 123
#if defined REPLACE_UNDEFINED && REPLACE_UNDEFINED == 123
    replace_undefined += 2;
#endif
    assert("replace_undefined", replace_undefined, 3);
  }

  return 0;
}

int function(int a, char b, struct Struct *c) {
  return a + b + c->i + c->j + c->k;
}
