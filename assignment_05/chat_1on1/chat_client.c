#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 100

int main()
{
    int sock;
    struct sockaddr_in server_address;
    char message[BUFFER_SIZE];

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        perror("socket() error");

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
        perror("connect() error");

    while (1)
    {
        // Get message from stdin
        printf("[Me] ");
        fgets(message, sizeof(message), stdin);

        // Write message to server
        write(sock, message, strlen(message));

        if (strcmp(message, "\\quit\n") == 0)
            break;

        // Get message from server
        int str_len = read(sock, message, sizeof(message) - 1);
        if (str_len <= 0)
            break;
        message[str_len] = 0;

        // Print message from server
        printf("[You] %s", message);
    }

    close(sock);
    return 0;
}
