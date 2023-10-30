#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (void) {
    printf("Going to sleep\n");
    sleep(1000); // sleep for 1000 seconds
    printf("Just woke up\n");
    return 0;
}