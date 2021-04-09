#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
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

assert 0 "return 0;"
assert 42 "return 42;"

assert 21 "return 5+20-4;"
assert 41 "return 12 + 34 - 5 ;"
assert 47 "return 5+6*7;"
assert 15 "return 5*(9-6);"
assert 4 "return (3+5)/2;"
assert 1 "return -1+2;"
assert 5 "return -3*+5+20;"

assert 1 "return 1==1;"
assert 0 "return 1==0;"
assert 0 "return 1!=1;"
assert 1 "return 1!=0;"
assert 0 "return 1<1;"
assert 1 "return 1<2;"
assert 0 "return 1>1;"
assert 1 "return 2>1;"
assert 0 "return 1<=0;"
assert 1 "return 1<=1;"
assert 0 "return 0>=1;"
assert 1 "return 1>=1;"

assert 1 "a = 1; return a;"
assert 2 "a = 1+2; a = a*2; return a/3;"
assert 3 "returnx = 1+2; return returnx;"

assert 1 "a = 1; if (a > 0) return 1; return 0;"
assert 0 "a = 0; if (a > 0) return 1; return 0;"
assert 1 "a = 1; if (a > 0) return 1; else return 0;"
assert 0 "a = 0; if (a > 0) return 1; else return 0;"

assert 55 "ans=0; for (i=1; i<=10; i=i+1) ans=ans+i; return ans;"
assert 10 "a=0; for (;;a=a+1) if (a>=10) return a;"

echo OK
