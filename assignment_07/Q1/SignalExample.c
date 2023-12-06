#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int sig)
{
    printf("Handler is called.\n");
}

int main()
{
    signal(SIGINT, handler);

    printf("Sleep begins!\n");
    sleep(1000);
    printf("Wake up!\n");

    return 0;
}
