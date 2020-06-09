#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int multi2mix(int a,int b) {return a * b; }
int sub(int x, int y) { return x-y; }
int add6(int a, int b, int c, int d, int e, int f) {
  return a+b+c+d+e+f;
}
EOF

assert() {
    expected="$1"
    input="$2"

    ./mcc "$input" > tmp.s
    gcc -static -o tmp tmp.s tmp2.o
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
assert 47 "return 5+6*7;"
assert 15 "return 5*(9-6);"
assert 0 "return -3*5+15;"
assert 0 "return 2*2!=4;"
assert 1 "return 5*(6+4)<89;"
assert 0 "return 67<35-25;"
assert 1 "return 6*7+3>=5*5;"
assert 60 "ai=3;b=4*5;ai = ai * b;return ai;"
assert 5 "if(7>5) return 5; else return 19;"
assert 10 "i = 0;while(i<10) i=i+1; return i;"
assert 165 "sum = 0;re = 0;for(i = 0;i < 10;i = i + 1){sum = sum + i; re = re + sum;} return re;"
assert 3 'return ret3();'
assert 5 'return ret5();'
assert 15 'return multi2mix(3,5);'
assert 21 'return add6(1,2,3,4,5,6);'
assert 2 'return sub(5, 3);'
echo OK