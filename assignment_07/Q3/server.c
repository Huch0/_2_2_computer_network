#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/select.h>

#define PORT 8080
#define TRUE 1
#define buffer_size 1024
#define N_CLIENT 3

void handler(int sig);
int maxArr(int arr[], int n);

int access_sock;

int main()
{
    signal(SIGINT, handler);

    int comm_sock[N_CLIENT];
    struct sockaddr_in addr;
    struct sockaddr_in client;
    socklen_t client_len;
    char buffer[buffer_size];

    int client_cnt = 0;

    access_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (access_sock == -1)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(access_sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(access_sock, 5) == -1)
    {
        perror("listen");
        exit(1);
    }

    for (int i = 0; i < N_CLIENT; i++)
    {
        if ((comm_sock[i] = accept(access_sock, (struct sockaddr *)&client, &client_len)) == -1)
        {
            perror("accept");
            exit(1);
        }
        else
            printf("Client #%d connected.\n", i + 1);
    }
    fd_set readfds;
    int ret;

    while (TRUE)
    {
        // Config fd_set
        FD_ZERO(&readfds);
        for (int i = 0; i < N_CLIENT; i++)
            FD_SET(comm_sock[i], &readfds);
        printf("waiting at select...\n");

        ret = select(maxArr(comm_sock, N_CLIENT) + 1, &readfds, NULL, NULL, NULL);
        printf("select returned: %d\n", ret);

        int i = 0;

        switch (ret)
        {
        case -1:
            perror("select");
            break;
        case 0:
            printf("select returns: 0\n");
            break;
        default:
            i = 0;

            while (ret > 0)
            {
                if (FD_ISSET(comm_sock[i], &readfds))
                {
                    memset(buffer, 0, buffer_size);

                    if (recv(comm_sock[i], buffer, buffer_size, 0) == -1)
                    {
                        perror("recv");
                        break;
                    }

                    ret--;
                    printf("MSG from Client %d: %s\n", i, buffer);
                }
                i++;
            }
            break;
        }
    }

    close(access_sock);
    return 0;
}

void handler(int sig)
{
    printf("Handler is called.\n");
    close(access_sock);
    exit(EXIT_SUCCESS);
}

int maxArr(int arr[], int n)
{
    int max = arr[0];
    for (int i = 1; i < n; i++)
        if (arr[i] > max)
            max = arr[i];
    return max;
}