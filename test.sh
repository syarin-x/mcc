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

assert 42 "return 42;"
assert 41 "return  12 + 34 - 5 ;"
assert 47 "return 5+6*7;"
assert 15 "return 5*(9-6);"
assert 0 "return -3*5+15;"
assert 0 "return 2*2!=4;"
assert 1 "return 5*(6+4)<89;"
assert 0 "return 67<35-25;"
assert 1 "return 6*7+3>=5*5;"
assert 60 "ai=3;b=4*5;ai = ai * b;return ai;"
assert 5 "if(7>5) return 5; return 19;"

echo OK