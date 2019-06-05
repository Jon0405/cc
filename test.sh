#!/bin/sh

try() {
  expected="$1"
  input="$2"

  ./cc "$input" > tmp.S
  gcc -o tmp tmp.S
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

tryfunc() {
  expected="$1"
  input="$2"

  ./cc "$input" > tmp.S
  gcc -c ./test/funccall.c
  gcc -o tmp tmp.S funccall.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 41 " 12 + 34 - 5 ;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 5 "-10+15;"
try 5 "20+(-3*+5);"
try 1 "2 > 1;"
try 0 "1 > 2;"
try 1 "1 >= 1;"
try 1 "42 == 42;"
try 0 "42 != 42;"
try 8 "a = 3; b = 5; a + b;"
try 8 "a = 3; b = 5; return a + b; return 42;"
try 8 "abc = 3; bcd = 5; return abc + bcd; return 42;"
try 8 "if (1) return 8; return 42;"
try 42 "if (0) return 8; return 42;"
try 8 "if (1) return 8; else return 42;"
try 42 "if (0) return 8; else return 42;"
try 8 "a = 10; if (a == 10) return 8; else return 42;"
try 42 "a = 0; if (a == 10) return 8; else return 42;"
try 10 "a = 0; while (a < 10) a = a + 1; return a;"
try 10 "b = 0; for (a = 0; a < 10; a = a + 1) b = b + 1; return b;"
try 20 "a = 0; for (b = 0; b < 10; b = b + 1) {a = a + 1; a = a + 1;} return a;"
tryfunc 3 "return foo(1, 2);"
tryfunc 3 "a = 1; b = 2; return foo(a, b);"

echo OK
