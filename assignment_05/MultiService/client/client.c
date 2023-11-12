// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080
#define TRUE 1
#define FALSE 0
#define buffer_size 1024

void download_file(int socket, char *file_name)
{
    // Open a file in binary mode for writing
    FILE *fp = fopen(file_name, "wb");

    if (fp == NULL)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // Receive the file size from the server
    long fsize;
    recv(socket, &fsize, sizeof(fsize), 0);

    // Buffer to store data
    char recv_buffer[buffer_size];

    // Receive data in chunks of buffer_size
    ssize_t bytes_received;
    long total_bytes_received = 0;
    while (total_bytes_received < fsize && (bytes_received = recv(socket, recv_buffer, buffer_size, 0)) > 0)
    {
        // Write the buffer to the file
        fwrite(recv_buffer, 1, bytes_received, fp);
        total_bytes_received += bytes_received;
    }

    if (bytes_received < 0)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    // Announce that the file has been downloaded
    printf("File downloaded successfully.\n");

    // Close the file
    fclose(fp);
}

int main(int argc, char const *argv[])
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char recv_buffer[buffer_size] = {0};
    char send_buffer[buffer_size] = {0};

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,
                          sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Client side: Waits for the server's response after sending a message
    while (1)
    {

        // Send a message to the server to indicate that the client is ready for the next menu message
        write(client_fd, "ready", strlen("ready"));

        // Read menu message from the server
        memset(recv_buffer, 0, buffer_size);
        ssize_t len = read(client_fd, recv_buffer, sizeof(recv_buffer) - 1);
        recv_buffer[len] = '\0'; // Add null character at the end
        printf("%s", recv_buffer);

        // Get message from stdin
        memset(send_buffer, 0, buffer_size);
        fgets(send_buffer, sizeof(send_buffer), stdin);

        if (strcmp(send_buffer, "1\n") == 0)
        {
            // Send "\\service 1" to the server
            memset(send_buffer, 0, buffer_size);
            strcpy(send_buffer, "\\service 1");
            write(client_fd, send_buffer, strlen(send_buffer));

            // Print the server's response
            memset(recv_buffer, 0, buffer_size);

            ssize_t len = read(client_fd, recv_buffer, sizeof(recv_buffer) - 1);
            recv_buffer[len] = '\0'; // Add null character at the end
            printf("%s\n", recv_buffer);
        }
        else if (strcmp(send_buffer, "2\n") == 0)
        {
            // Send "\\service 2" to the server
            memset(send_buffer, 0, buffer_size);
            strcpy(send_buffer, "\\service 2");
            write(client_fd, send_buffer, strlen(send_buffer));

            // Print file list from the server
            memset(recv_buffer, 0, buffer_size);

            ssize_t len = read(client_fd, recv_buffer, sizeof(recv_buffer) - 1);
            recv_buffer[len] = '\0'; // Add null character at the end
            printf("%s", recv_buffer);

            // Get message from stdin (1, 2, 3)
            memset(send_buffer, 0, buffer_size);
            fgets(send_buffer, sizeof(send_buffer), stdin);

            // Send message to the server (1, 2, 3)
            write(client_fd, send_buffer, strlen(send_buffer));

            if (strcmp(send_buffer, "1\n") == 0)
            {
                // Download file 1. Book.txt
                download_file(client_fd, "Book.txt");
            }
            else if (strcmp(send_buffer, "2\n") == 0)
            {
                // Download file 2. Linux.png
                download_file(client_fd, "Linux.png");
            }
        }
        else
        {
            // Send "\\service 3" to the server
            memset(send_buffer, 0, buffer_size);
            strcpy(send_buffer, "\\service 3");
            write(client_fd, send_buffer, strlen(send_buffer));

            while (1)
            {
                // Send message to the server
                memset(send_buffer, 0, buffer_size);
                fgets(send_buffer, sizeof(send_buffer), stdin);
                write(client_fd, send_buffer, strlen(send_buffer));

                // Print the server's response
                memset(recv_buffer, 0, buffer_size);

                ssize_t len = read(client_fd, recv_buffer, sizeof(recv_buffer) - 1);
                recv_buffer[len] = '\0'; // Add null character at the end
                printf("[You] %s", recv_buffer);

                if (strcmp(recv_buffer, "\\quit\n") == 0)
                {
                    break;
                }
            }
        }
    }

    // closing the connected socket
    close(client_fd);

    return 0;
}
