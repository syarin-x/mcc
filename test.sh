#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./mcc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input -> $actual"
    else
        echo "$input -> $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"
assert 35 "1+2+30+2;"
assert 35 "30+10-5;"
assert 35 " 30 + 10 - 5 ;"
assert 41 " 12 + 34 - 5 ;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 0 "-3*5+15;"
assert 1 "1==1;"
assert 0 "1==0;"
assert 1 "1+2!=36;"
assert 0 "2*2!=4;"
assert 1 "5*(6+4)<89;"
assert 0 "67<35-25;"
assert 1 "78<=78;"
assert 1 "79<=120*2;"
assert 1 "6*7+3>=5*5;"
assert 1 "a=3;a>2;"

echo OK