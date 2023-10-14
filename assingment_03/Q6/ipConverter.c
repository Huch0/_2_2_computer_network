#include <stdio.h>

// Convert a decimal number to 8-bit binary representation
void decToBin(int dec, char binStr[]) {
    for (int i = 7; i >= 0; i--) {
        binStr[i] = (dec & 1) + '0';
        dec = dec >> 1;
    }
    binStr[8] = '\0';
}

int main() {
    int a, b, c, d;
    char binA[9], binB[9], binC[9], binD[9];
    scanf("%d.%d.%d.%d", &a, &b, &c, &d);

    decToBin(a, binA);
    decToBin(b, binB);
    decToBin(c, binC);
    decToBin(d, binD);

    printf("%s.%s.%s.%s\n", binA, binB, binC, binD);

    return 0;
}
