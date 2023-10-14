#include <stdio.h>

struct info {int n;};

void callByVal (struct info info) {
    info.n = 20;
}

void callByRef (struct info* info) {
    info->n = 30;
}

int main () {
    struct info myinfo;

    myinfo.n = 10;
    printf("%d\n", myinfo.n);

    callByVal(myinfo);
    printf("%d\n", myinfo.n);

    myinfo.n = 10;
    printf("%d\n", myinfo.n);

    callByRef(&myinfo);
    printf("%d\n", myinfo.n);

    return 0;
}