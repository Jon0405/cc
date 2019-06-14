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
  gcc -c ./test/$func.c
  gcc -o tmp tmp.S $func.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 14 "int main() {return 4 * 3 + 2;}"
try 42 "int main() {return 42;}"
try 42 "int main() {;;;return 42;}"
try 10 "int main() {int i; for (i = 0; i < 10; i = i + 1); return i;}"
try 42 "int main() {if (1) return 42; return 8;}"
try 8 "int main() {if (0) return 42; return 8;}"
try 42 "int main() {if (1) return 42; else return 8;}"
try 8 "int main() {if (0) return 42; else return 8;}"
func="funccall"; tryfunc 3 "int main() {return foo(1, 2);}"
func="funccall"; tryfunc 3 "int main() {int a = 1; int b = 2; return foo(a, b);}"
try 3 "int main() {return foo();} int foo() {return 3;}"
try 3 "int main() {return foo(1, 2);} int foo(int a, int b) {return a + b;}"
try 10 "int main() {int i; int a = 0; for (i = 0; i < 10; i = i + 1) a = a + 1; return a;}"
try 8 "int main() {return fib(6);} int fib(int a) {if (a == 0) return 0; if (a == 1) return 1; return fib(a-1) + fib(a-2);}"
try 10 "int main() {int a = 0; while (a < 10) a = a + 1; return a;}"
try 6 "int main() {int a; int b; a = b = 3; return a + b;}"
try 8 "int main() {int a = 8; int *b = &a; return *b;}"
try 42 "int main() {int a = 8; int *b = &a; *b = 42; return a;}"
try 42 "int main() {int a = 8; int *b = &a; int **c = &b; **c = 42; return a;}"
func="ptr"; tryfunc 3 "int main() {int *a = arr(10); int *b = a + 4; return *b;}"
func="ptr"; tryfunc 5 "int main() {int *a = arr(10); int *b = a + 5; return *b;}"

echo OK
