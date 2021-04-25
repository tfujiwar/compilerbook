#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./mycc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "int main() { return 0; }"
assert 42 "int main() { return 42; }"

assert 21 "int main() { return 5+20-4; }"
assert 41 "int main() { return 12 + 34 - 5 ; }"
assert 47 "int main() { return 5+6*7; }"
assert 15 "int main() { return 5*(9-6); }"
assert 4 "int main() { return (3+5)/2; }"
assert 1 "int main() { return -1+2; }"
assert 5 "int main() { return -3*+5+20; }"

assert 1 "int main() { return 1==1; }"
assert 0 "int main() { return 1==0; }"
assert 0 "int main() { return 1!=1; }"
assert 1 "int main() { return 1!=0; }"
assert 0 "int main() { return 1<1; }"
assert 1 "int main() { return 1<2; }"
assert 0 "int main() { return 1>1; }"
assert 1 "int main() { return 2>1; }"
assert 0 "int main() { return 1<=0; }"
assert 1 "int main() { return 1<=1; }"
assert 0 "int main() { return 0>=1; }"
assert 1 "int main() { return 1>=1; }"

assert 1 "int main() { return 1 && 1; }"
assert 0 "int main() { return 1 && 0; }"
assert 0 "int main() { return 0 && 1; }"
assert 0 "int main() { return 0 && 0; }"
assert 1 "int main() { int a=1; int b=1; (a=a-1) && (b=b-1); return b; }"
assert 1 "int main() { return 1 || 1; }"
assert 1 "int main() { return 1 || 0; }"
assert 1 "int main() { return 0 || 1; }"
assert 0 "int main() { return 0 || 0; }"
assert 0 "int main() { int a=0; int b=0; (a=a+1) || (b=b+1); return b; }"

assert 0 "int main() { return 1 & 2; }"
assert 3 "int main() { return 1 | 2; }"
assert 2 "int main() { return 1 ^ 3; }"
assert 31 "int main() { return 1 | 2 | 4 | 8 | 16; }"

assert 4 "int main() { return 1 << 2; }"
assert 1 "int main() { return 4 >> 2; }"
assert 4 "int main() { return 4 >> 1 >> 1 << 1 << 1; }"

assert 1 "int main() { int a; char b; a=1; b=1; return a==b; }"
assert 0 "int main() { int a; char b; a=1; b=0; return a==b; }"
assert 0 "int main() { int a; char b; a=1; b=1; return a!=b; }"
assert 1 "int main() { int a; char b; a=1; b=0; return a!=b; }"
assert 0 "int main() { int a; char b; a=1; b=1; return a<b; }"
assert 1 "int main() { int a; char b; a=1; b=2; return a<b; }"
assert 0 "int main() { int a; char b; a=1; b=1; return a>b; }"
assert 1 "int main() { int a; char b; a=2; b=1; return a>b; }"
assert 0 "int main() { int a; char b; a=1; b=0; return a<=b; }"
assert 1 "int main() { int a; char b; a=1; b=1; return a<=b; }"
assert 0 "int main() { int a; char b; a=0; b=1; return a>=b; }"
assert 1 "int main() { int a; char b; a=1; b=1; return a>=b; }"

assert 1 "int main() { char a; int b; a=1; b=1; return a==b; }"
assert 0 "int main() { char a; int b; a=1; b=0; return a==b; }"
assert 0 "int main() { char a; int b; a=1; b=1; return a!=b; }"
assert 1 "int main() { char a; int b; a=1; b=0; return a!=b; }"
assert 0 "int main() { char a; int b; a=1; b=1; return a<b; }"
assert 1 "int main() { char a; int b; a=1; b=2; return a<b; }"
assert 0 "int main() { char a; int b; a=1; b=1; return a>b; }"
assert 1 "int main() { char a; int b; a=2; b=1; return a>b; }"
assert 0 "int main() { char a; int b; a=1; b=0; return a<=b; }"
assert 1 "int main() { char a; int b; a=1; b=1; return a<=b; }"
assert 0 "int main() { char a; int b; a=0; b=1; return a>=b; }"
assert 1 "int main() { char a; int b; a=1; b=1; return a>=b; }"

assert 1 "int main() { int a; a=1; return a; }"
assert 2 "int main() { int a; a=1+2; a=a*2; return a/3; }"
assert 3 "int main() { int returnx; returnx=1+2; return returnx; }"

assert 1 "int main() { int a; a=1; if (a>0) return 1; return 0; }"
assert 0 "int main() { int a; a=0; if (a>0) return 1; return 0; }"
assert 1 "int main() { int a; a=1; if (a>0) return 1; else return 0; }"
assert 0 "int main() { int a; a=0; if (a>0) return 1; else return 0; }"

assert 55 "int main() { int ans; int i; ans=0; for (i=1; i<=10; i=i+1) ans=ans+i; return ans; }"
assert 10 "int main() { int a; a=0; for (;;a=a+1) if (a>=10) return a; }"

assert 10 "int main() { int a; a=0; while (a<10) a=a+1; return a; }"
assert 10 "int main() { int a; a=0; while (a=a+1) if (a>=10) return a; }"

assert 2 "int main() { int a; { a=1; a=a+1; } return a; }"
assert 4 "int main() { int a; { a=1; {a=a+1; a=a+1; } a=a+1; } return a; }"
assert 1 "int main() { int a; a=1; if (a==1) { return 1; } else { return 0; } }"
assert 0 "int main() { int a; a=0; if (a==1) { return 1; } else { return 0; } }"

assert 3 "int f(int a, int b) { return a+b; } int main() { return f(1,2); }"
assert 10 "int f(int a, int b) { return a+b; } int main() { return f(f(1,2), f(3,4)); }"
assert 10 "int f(char a, char b) { return a+b; } int main() { return f(f(1,2), f(3,4)); }"
assert 10 "int f(int a, char b) { return a+b; } int main() { return f(f(1,2), f(3,4)); }"
assert 3 "int f(int a, int b) { return a+b; } int main() { int a; int b; a=1; b=2; return f(a,b); }"
assert 3 "int f(int *a, int *b) { return *a+*b; } int main() { int a; int b; a=1; b=2; return f(&a,&b); }"

assert 3 "int main() { int x; int *y; x=3; y=&x; return *y; }"
assert 3 "int main() { int x; int *y; y=&x; *y=3; return x; }"
assert 3 "int main() { int x; int *y; int **z; y=&x; z=&y; **z=3; return x; }"

assert 4 "int main() { int x; return sizeof(x); }"
assert 4 "int main() { int x; return sizeof(x+1); }"
assert 4 "int main() { int x; return sizeof(1+x); }"
assert 8 "int main() { int *x; return sizeof(x); }"
assert 8 "int main() { int *x; return sizeof(x+1); }"
assert 8 "int main() { int *x; return sizeof(1+x); }"
assert 4 "int main() { int a[10]; return sizeof(a[0]); }"
assert 8 "int main() { int a[10]; return sizeof(&a); }"
assert 8 "int main() { int a[10]; return sizeof(a+1); }"
assert 40 "int main() { int a[10]; return sizeof(a); }"

assert 3 "int main() { int a[2]; *a=1; *(a+1)=2; int *p; p=a; return *p+*(p+1); }"
assert 3 "int main() { int a[2]; *a=1; *(a+1)=2; int *p; p=a+1; return *p+*(p-1); }"
assert 3 "int main() { int a[2]; a[0]=1; a[1]=2; return a[0]+a[1]; }"

assert 3 "int main() { int a[2]; *(a+1)=1; return 3; }"
assert 3 "int main() { int a[2]; a[1]=1; return 3; }"

assert 0 "int a; int main() { return a; }"
assert 1 "int a; int main() { a=1; return a; }"
assert 3 "int a[2]; int main() { a[0]=1; a[1]=2; return a[0]+a[1]; }"

assert 1 "int main() { char a; a=1; return a; }"
assert 3 "int main() { char a; char b; a=1; b=2; return a+b; }"
assert 1 "int main() { char a; char b; a=1; b=2; return b-a; }"
assert 3 "int main() { char a; int b; a=1; b=2; return a+b; }"
assert 1 "int main() { char a; int b; a=1; b=2; return b-a; }"
assert 3 "int main() { char a[2]; a[0]=1; a[1]=2; return a[0]+a[1]; }"
assert 1 "int main() { char a; return sizeof(a); }"
assert 2 "int main() { char a[2]; return sizeof(a); }"
assert 8 "int main() { char a; return sizeof(&a); }"
assert 1 "int main() { char a; a=1; int b; b=a; return b; }"
assert 1 "int main() { int a; a=1; char b; b=a; return b; }"
assert 1 "int main() { char a; char b; a=-1; b=2; return a+b; }"
assert 1 "int main() { char a; char b; a=128; b=-128; return a==b; }"
assert 1 "int main() { int a; int b; a=128; b=-128; return a!=b; }"

assert 65 "int main() { char *s; s=\"ABC\"; return s[0]; }"
assert 66 "int main() { char *s; s=\"ABC\"; return s[1]; }"
assert 67 "int main() { char *s; s=\"ABC\"; return s[2]; }"
assert 0 "int main() { char *s; s=\"ABC\"; return s[3]; }"

assert 11 "int a; int main() { int a; a=10; { int a; a=100; a=a+1; } a=a+1; return a; }"
assert 101 "int a; int main() { int a; a=10; { int a; a=100; a=a+1; return a; } a=a+1; }"

assert 1 "int main() { int a=1; return a; }"
assert 2 "int main() { int a=1; int b=a*2; return b; }"
assert 1 "int main() { int a[]={1,2,3}; return a[0]; }"
assert 1 "int main() { int a[5]={1,2,3}; return a[0]; }"
assert 2 "int main() { int a[5]={1,2,3}; return a[1]; }"
assert 3 "int main() { int a[5]={1,2,3}; return a[2]; }"
assert 0 "int main() { int a[5]={1,2,3}; return a[3]; }"
assert 0 "int main() { int a[5]={1,2,3}; return a[4]; }"
assert 65 "int main() { char a[]=\"ABC\"; return a[0]; }"
assert 66 "int main() { char a[]=\"ABC\"; return a[1]; }"
assert 67 "int main() { char a[]=\"ABC\"; return a[2]; }"
assert 0 "int main() { char a[]=\"ABC\"; return a[3]; }"
assert 4 "int main() { char a[]=\"ABC\"; return sizeof(a); }"
assert 65 "int main() { char a[5]=\"ABC\"; return a[0]; }"
assert 66 "int main() { char a[5]=\"ABC\"; return a[1]; }"
assert 67 "int main() { char a[5]=\"ABC\"; return a[2]; }"
assert 0 "int main() { char a[5]=\"ABC\"; return a[3]; }"
assert 0 "int main() { char a[5]=\"ABC\"; return a[4]; }"
assert 5 "int main() { char a[5]=\"ABC\"; return sizeof(a); }"

assert 1 "int a=1; int main() { return a; }"
assert 2 "int a=1; int main() { int b=a*2; return b; }"
assert 1 "int a[]={1,2,3}; int main() { return a[0]; }"
assert 1 "int a[5]={1,2,3}; int main() { return a[0]; }"
assert 2 "int a[5]={1,2,3}; int main() { return a[1]; }"
assert 3 "int a[5]={1,2,3}; int main() { return a[2]; }"
assert 0 "int a[5]={1,2,3}; int main() { return a[3]; }"
assert 0 "int a[5]={1,2,3}; int main() { return a[4]; }"
assert 65 "char a[]=\"ABC\"; int main() { return a[0]; }"
assert 66 "char a[]=\"ABC\"; int main() { return a[1]; }"
assert 67 "char a[]=\"ABC\"; int main() { return a[2]; }"
assert 0 "char a[]=\"ABC\"; int main() { return a[3]; }"
assert 4 "char a[]=\"ABC\"; int main() { return sizeof(a); }"
assert 65 "char a[5]=\"ABC\"; int main() { return a[0]; }"
assert 66 "char a[5]=\"ABC\"; int main() { return a[1]; }"
assert 67 "char a[5]=\"ABC\"; int main() { return a[2]; }"
assert 0 "char a[5]=\"ABC\"; int main() { return a[3]; }"
assert 0 "char a[5]=\"ABC\"; int main() { return a[4]; }"
assert 5 "char a[5]=\"ABC\"; int main() { return sizeof(a); }"
assert 4 "char s[]=\"ABC\"; int main() { return printf(\"%s\n\", s); }"

assert 0 "int main() { return 0; } // comment"
assert 0 "int main() { int a=0; /* a=1 */ return a; }"

assert 4 "int main() { int a=2; a+=1+1; return a; }"
assert 0 "int main() { int a=2; a-=1+1; return a; }"
assert 4 "int main() { int a=2; a*=1+1; return a; }"
assert 1 "int main() { int a=2; a/=1+1; return a; }"
assert 2 "int main() { int a; int b; a=b=1; return a+b; }"
assert 12 "int main() { int a=1; int b=10; a+=b+=1; return a; }"

assert 2 "int main() { int a=0; int b=++a; return a+b; }"
assert 0 "int main() { int a=1; int b=--a; return a+b; }"
assert 1 "int main() { int a=0; return !a; }"
assert 0 "int main() { int a=1; return !a; }"
assert 0 "int main() { int a=-1; return ~a; }"

echo OK
