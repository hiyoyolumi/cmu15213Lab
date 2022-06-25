#include <stdio.h>

int func4(int input, int b, int c) {
    int val = (b + c) / 2;
    if (val <= input) {
        if (val >= input) {
            return 0;
        }
        int res = 2 * func4(input, val + 1, c) + 1;
        return res;
    } else {
        int res = 2 * func4(input, b, val - 1);
        return res;
    }
}

int main() {
    for (int i = 0; i <= 14; i++) {
        if (!func4(i, 0, 14)) {
            printf("%d\n", i);
        }
    }
    return 0;
}