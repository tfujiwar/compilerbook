#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s lib.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "main() { return 0; }"
assert 42 "main() { return 42; }"

assert 21 "main() { return 5+20-4; }"
assert 41 "main() { return 12 + 34 - 5 ; }"
assert 47 "main() { return 5+6*7; }"
assert 15 "main() { return 5*(9-6); }"
assert 4 "main() { return (3+5)/2; }"
assert 1 "main() { return -1+2; }"
assert 5 "main() { return -3*+5+20; }"

assert 1 "main() { return 1==1; }"
assert 0 "main() { return 1==0; }"
assert 0 "main() { return 1!=1; }"
assert 1 "main() { return 1!=0; }"
assert 0 "main() { return 1<1; }"
assert 1 "main() { return 1<2; }"
assert 0 "main() { return 1>1; }"
assert 1 "main() { return 2>1; }"
assert 0 "main() { return 1<=0; }"
assert 1 "main() { return 1<=1; }"
assert 0 "main() { return 0>=1; }"
assert 1 "main() { return 1>=1; }"

assert 1 "main() { a = 1; return a; }"
assert 2 "main() { a = 1+2; a = a*2; return a/3; }"
assert 3 "main() { returnx = 1+2; return returnx; }"

assert 1 "main() { a = 1; if (a > 0) return 1; return 0; }"
assert 0 "main() { a = 0; if (a > 0) return 1; return 0; }"
assert 1 "main() { a = 1; if (a > 0) return 1; else return 0; }"
assert 0 "main() { a = 0; if (a > 0) return 1; else return 0; }"

assert 55 "main() { ans=0; for (i=1; i<=10; i=i+1) ans=ans+i; return ans; }"
assert 10 "main() { a=0; for (;;a=a+1) if (a>=10) return a; }"

assert 10 "main() { a=0; while (a<10) a=a+1; return a; }"
assert 10 "main() { a=0; while (a=a+1) if (a>=10) return a; }"

assert 2 "main() { { a=1; a=a+1; } return a; }"
assert 4 "main() { { a=1; {a=a+1; a=a+1; } a=a+1; } return a; }"
assert 1 "main() { a=1; if (a==1) { return 1; } else { return 0; } }"
assert 0 "main() { a=0; if (a==1) { return 1; } else { return 0; } }"

assert 0 "main() { return foo(); }"
assert 10 "main() { return bar(bar(1, 2), bar(3, 4)); }"
assert 10 "f(a,b) { return a+b; } main() { return f(f(1,2), bar(3,4)); }"

assert 3 "main() { x=3; y=&x; return *y; }"
assert 3 "main() { x=3; y=5; z=&y+8; return *z; }"

echo OK
