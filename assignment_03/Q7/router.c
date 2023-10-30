#include <stdio.h>

void andWithMask (char input[4], char mask[4], char result[4]) {
    for (int i = 0; i < 4; i++) {
        result[i] = input[i] & mask[i];
    }
}

int isEqual (char result[4], char dest[4]) {
    for (int i = 0; i < 4; i++) {
        if (result[i] != dest[i])
            return 0;
    }
    return 1;
}

int main () {
    // list of subnet masks
    char mask1[4] = {255, 255, 255, 0};
    char mask2[4] = {255, 255, 0, 0};
    char mask3[4] = {0, 0, 0, 0};
    // list of destinations
    char dest1[4] = {10, 0, 2, 0};
    char dest2[4] = {192, 168, 0, 0};
    char dest3[4] = {0, 0, 0, 0};
    // list of masking results
    char maskingResult1[4] = {0, 0, 0, 0};
    char maskingResult2[4] = {0, 0, 0, 0};
    char maskingResult3[4] = {0, 0, 0, 0};

    char inputIP[4] = {0, 0, 0, 0};

    scanf("%d.%d.%d.%d", &inputIP[0], &inputIP[1], &inputIP[2] , &inputIP[3]);

    // Implement and operation with masks
    andWithMask(inputIP, mask1, maskingResult1);
    andWithMask(inputIP, mask2, maskingResult2);
    andWithMask(inputIP, mask3, maskingResult3);

    if (isEqual(maskingResult1, dest1) == 1) 
        printf("Send to IF0");
    else if (isEqual(maskingResult2, dest2) == 1)
        printf("Send to IF1");
    else if (isEqual(maskingResult3, dest3) == 1)
        printf("Send to IF2");
    else
        printf("Send to default interface");

    return 0;
}