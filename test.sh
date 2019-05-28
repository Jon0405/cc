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

echo OK
