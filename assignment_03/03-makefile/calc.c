#include <stdio.h>
#include <stdlib.h>
int extern getSum(int a, int b);

int main (void) {
    int a = 10;
    int b = 5;

    int sum = getSum(a, b);

    printf("Sum = %d\n", sum);
    return 0;
}