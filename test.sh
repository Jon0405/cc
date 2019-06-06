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
    echo "$input => $expected expected, but got $actual"
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

try 42 "int main() {return 42;}"
try 42 "int main() {if (1) return 42; return 8;}"
tryfunc 3 "int main() {return foo(1, 2);}"
tryfunc 3 "int main() {int a = 1; int b = 2; return foo(a, b);}"
try 3 "int main() {return foo();} int foo() {return 3;}"
try 3 "int main() {return foo(1, 2);} int foo(int a, int b) {return a + b;}"
try 10 "int main() {int i; int a = 0; for (i = 0; i < 10; i = i + 1) a = a + 1; return a;}"
try 8 "int main() {return fib(6);} int fib(int a) {if (a == 0) return 0; if (a == 1) return 1; return fib(a-1) + fib(a-2);}"
try 10 "int main() {int a = 0; while (a < 10) a = a + 1; return a;}"
try 6 "int main() {int a; int b; a = b = 3; return a + b;}"

echo OK
