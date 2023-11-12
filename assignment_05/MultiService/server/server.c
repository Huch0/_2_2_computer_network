// Server side C/C++ program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h> /* time_t, struct tm, time, localtime */

#define PORT 8080
#define TRUE 1
#define FALSE 0
#define buffer_size 1024

void send_current_time(int socket)
{
    char send_buffer[buffer_size] = {0};
    time_t rawtime;
    struct tm *timeinfo;

    // Get current time
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Send timeinfo to the client as a string format
    memset(send_buffer, 0, buffer_size);
    strcpy(send_buffer, asctime(timeinfo));
    send(socket, send_buffer, strlen(send_buffer), 0);
}

void send_file(int socket, char *file_name)
{
    // Open the file in binary mode
    FILE *fp = fopen(file_name, "rb");

    if (fp == NULL)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // Get the size of the file
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET); // same as rewind(f);

    // Send the file size to the client
    send(socket, &fsize, sizeof(fsize), 0);

    // Buffer to store data
    char send_buffer[buffer_size];

    // Read the file into the buffer and send it to the client in chunks of buffer_size
    ssize_t bytes_read;
    while ((bytes_read = fread(send_buffer, 1, buffer_size, fp)) > 0)
    {
        send(socket, send_buffer, bytes_read, 0);
    }

    // Close the file
    fclose(fp);
}

void echo_server(int socket)
{
    char recv_buffer[buffer_size] = {0};

    while (1)
    {
        // Read the client's message
        memset(recv_buffer, 0, buffer_size);

        ssize_t len = read(socket, recv_buffer, sizeof(recv_buffer) - 1);
        recv_buffer[len] = '\0'; // Add null character at the end

        // Send the client's message back to the client
        send(socket, recv_buffer, strlen(recv_buffer), 0);

        // If the client sends "\\quit", break the loop
        if (strcmp(recv_buffer, "\\quit\n") == 0)
            break;
    }
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char send_buffer[buffer_size] = {0};
    char recv_buffer[buffer_size] = {0};
    char *menu_message = "[Service List]\n1. Get Current Time\n2. Download File\n3. Echo Server\nEnter: ";
    char *file_list = "[Available File List]\n1. Book.txt\n2. Linux.png\n3. Go back\nEnter: ";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             &addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Wait for the client to indicate that it's ready for the next menu message
        memset(recv_buffer, 0, buffer_size);
        read(new_socket, recv_buffer, sizeof(recv_buffer) - 1);

        // Check if the client is ready for the next menu message
        if (strcmp(recv_buffer, "ready") == 0)
        {
            // Send menu message to the client
            send(new_socket, menu_message, strlen(menu_message), 0);
        }

        // Read the client's message
        memset(recv_buffer, 0, buffer_size);
        ssize_t len = read(new_socket, recv_buffer, sizeof(recv_buffer) - 1);
        recv_buffer[len] = '\0'; // Add null character at the end

        if (strcmp(recv_buffer, "\\service 1") == 0)
        {
            // Service 1 : Send Current Time
            send_current_time(new_socket);
        }
        else if (strcmp(recv_buffer, "\\service 2") == 0)
        {
            // Service 2 : Download File

            // Send file list to the client
            send(new_socket, file_list, strlen(file_list), 0);

            // Read the client's message
            memset(recv_buffer, 0, buffer_size);
            ssize_t len = read(new_socket, recv_buffer, sizeof(recv_buffer) - 1);
            recv_buffer[len] = '\0'; // Add null character at the end

            if (strcmp(recv_buffer, "1\n") == 0)
            {
                // Send Book.txt to the client
                send_file(new_socket, "Book.txt");
            }
            else if (strcmp(recv_buffer, "2\n") == 0)
            {
                // Send Linux.png to the client
                send_file(new_socket, "Linux.png");
            }
        }
        else if (strcmp(recv_buffer, "\\service 3") == 0)
        {
            // Service 3 : Echo Server
            echo_server(new_socket);
        }
    }

    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    close(server_fd);
    return 0;
}
