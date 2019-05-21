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

try 0 0
try 42 42
try 21 '5+20-4'

echo OK
