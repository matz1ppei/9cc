##! /bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
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
try 21 "5+20-4"
try 41 " 12 + 34 -5 "
try 7  "1+2*3"
try 5  "3+6/3"
try 15 "5*(9-6)"
try 4  "(3+5)/2"
try 2  "10-(3+5)"
try 5  "20-3*+5"
try 18 "6*(+3)"
try 20 "60+20*(-2)"
try 4  "7-15/5"
try 0  "1==2"
try 1  "(1+2)==(2+1)"
try 1  "3!=2"
try 0  "(5+4)!=(3+6)"
try 0  "5<4"
try 1  "(1+2)<7"
try 0  "10<=9"
try 1  "8<=(11-1)"
try 0  "9>11"
try 1  "8*2>10"
try 0  "13>=20"
try 1  "12/4>=2"
try 31 "((((((((((((((((((((((((((((((1+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)+1)"

echo OK