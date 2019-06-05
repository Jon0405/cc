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

try 42 "main() {return 42;}"
tryfunc 3 "main() {return foo(1, 2);}"
tryfunc 3 "main() {a = 1; b = 2; return foo(a, b);}"
try 3 "main() {return foo();} foo() {return 3;}"
try 3 "main() {return foo(1, 2);} foo(a, b) {return a + b;}"

echo OK
