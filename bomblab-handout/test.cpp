#include <iostream>
#include <random>
using namespace std;

int func4(int a, int b, int c) {
    int d = c;
    d -= b;
    d += (d < 0) ? 1 : 0;
    d >>= 1;
    int f = b + d;
    if(f <= a) {
        d = 0;
        if(f >= a) {
            return d;
        } else {
            b = f+1;
            return 2*func4(a,b,c)+1;
        }
    }
    c = f-1;
    return 2*func4(a,b,c);
}

int main() {
    cout << rand() % 2 << endl;
    return 0;
}