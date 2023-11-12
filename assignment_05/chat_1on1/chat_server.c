#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 100

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_size;
    char message[BUFFER_SIZE];

    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
        perror("socket() error");

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
        perror("bind() error");

    if (listen(server_socket, 5) == -1)
        perror("listen() error");

    client_address_size = sizeof(client_address);
    client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);
    if (client_socket == -1)
        perror("accept() error");
    else
        puts("Connected client");

    while (1) {
        // Read message from client
        int str_len = read(client_socket, message, sizeof(message) - 1);
        if (str_len <= 0)
            break;
        message[str_len] = 0;

        // Print message from client
        printf("[You] %s", message);

        // Quit if message is "\quit"
        if (strcmp(message, "\\quit\n") == 0)
            break;

        // Get message from stdin
        printf("[Me] ");
        fgets(message, sizeof(message), stdin);

        // Write message to client
        write(client_socket, message, strlen(message));
    }

    close(client_socket);
    close(server_socket);
    return 0;
}