#include <stdio.h>
#include <stdlib.h>

int main () {
    int n = 0;
    scanf("%d", &n);

    // Allocate memory for an array of n integers
    int* numArray = (int*)calloc(n, sizeof(int));

    // Store 1 ~ n to array
    for (int i = 0; i < n; i++) {
        numArray[i] = i + 1;
    }

    // Sum 1 ~ n
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += numArray[i];
    }

    printf("%d", sum);
    
    free(numArray);

    return 0;
}