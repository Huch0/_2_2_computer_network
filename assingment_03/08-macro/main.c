#include <stdio.h>
#include <stdlib.h>

int main(void) {

#ifdef DEBUG
    printf("This is my debug message ... \n");
#endif

    printf("Hello world\n");

#ifdef DEBUG
    printf("Debugging never stops ... \n");
#endif

    return 0;
}