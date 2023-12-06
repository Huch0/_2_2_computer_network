#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PORT 8080
#define TRUE 1
#define buffer_size 1024

void handler(int sig);

int sd;

int main()
{
    signal(SIGINT, handler);

    char buffer[buffer_size];
    struct sockaddr_in sin;

    memset((char *)&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
        perror("connect");
        exit(1);
    }

    while (TRUE)
    {
        printf("Enter: ");
        fgets(buffer, buffer_size, stdin);

        if (send(sd, buffer, strlen(buffer), 0) == -1)
        {
            perror("send");
            exit(1);
        }

        if (strncmp(buffer, "quit", 4) == 0)
        {
            break;
        }
    }

    close(sd);

    return 0;
}

void handler(int sig)
{
    printf("Handler is called.\n");
    close(sd);
    exit(EXIT_SUCCESS);
}