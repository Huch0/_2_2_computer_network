#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PORT 8080
#define TRUE 1
#define buffer_size 1024

void echo_server(int socket);
void handler(int sig);

int sd;

int main()
{
    signal(SIGINT, handler);

    int new;
    struct sockaddr_in addr;
    struct sockaddr_in client;
    socklen_t client_len;
    int client_cnt = 0;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(sd, 5) == -1)
    {
        perror("listen");
        exit(1);
    }

    while ((new = accept(sd, (struct sockaddr *)&client, &client_len)) != -1)
    {
        if (fork() == 0)
        {
            // Child process
            close(sd);
            echo_server(new);
            close(new);
            exit(0);
        }
        // Parent process
        close(new);

        client_cnt++;
        printf("New Client!\n");
        printf("Number of service client : %d\n", client_cnt);
    }

    close(sd);
    return 0;
}

void echo_server(int socket)
{
    char buffer[buffer_size];
    ssize_t bytes_read;

    while (TRUE)
    {
        // Read data from the client
        bytes_read = recv(socket, buffer, buffer_size, 0);

        if (bytes_read == 0)
        {
            // Client disconnected
            break;
        }

        if (bytes_read == -1)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        printf("Recv from Client: %s", buffer);

        // Send the data back to the client
        send(socket, buffer, bytes_read, 0);

        if (strncmp(buffer, "quit", 4) == 0)
        {
            break;
        }
    }

    close(socket);
}

void handler(int sig)
{
    printf("Handler is called.\n");
    close(sd);
    exit(EXIT_SUCCESS);
}