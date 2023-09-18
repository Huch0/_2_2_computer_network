#include <stdio.h>
#include <stdlib.h>

int main () {
    int sum = 0;

    for (int i = 0; i < 5; i++) {
        int input = 0;
        scanf("%d", &input);
        sum += input;
    }

    printf("SUM: %d\n", sum); 

    return 0;
}