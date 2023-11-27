#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h> // Include for close()

#include <netinet/in.h> // Include for INET sockets

#include <fcntl.h> // Include for fcntl()

#include <errno.h> // Include for errno

#define SERVER_PATH "./local_server1"
#define BUFFER_LENGTH 250
#define TRUE 1
#define FALSE 0

#define PORT 8080 // Port number for INET socket

int main()
{
    // UNIX
    int sd = -1, sd2 = -1;
    int rc, length;
    char buffer[BUFFER_LENGTH];
    struct sockaddr_un serveraddr;

    struct sockaddr_un clientaddr;
    socklen_t clientaddr_len;

    // INET Server
    int inet_server_fd, inet_new_socket;
    struct sockaddr_in inet_address;
    int inet_opt = 1;
    socklen_t inet_addrlen = sizeof(inet_address);
    char inet_buffer[BUFFER_LENGTH];

    do
    {
        sd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sd < 0)
        {
            perror("socket() failed");
            break;
        }

        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sun_family = AF_UNIX;
        strcpy(serveraddr.sun_path, SERVER_PATH);

        rc = bind(sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
        if (rc < 0)
        {
            perror("bind() failed");
            break;
        }

        rc = listen(sd, 10);
        if (rc < 0)
        {
            perror("listen() failed");
            break;
        }

        printf("[Info] Unix socket : waiting for conn req\n");

        // Try until success
        while (sd2 < 0)
        {
            sd2 = accept(sd, (struct sockaddr *)&clientaddr, &clientaddr_len);
            if (sd2 < 0)
            {
                perror("accept() failed");
            }
        }

        printf("[Info] Unix socket : client connected\n");

        // Open INET server.// Creating socket file descriptor
        if ((inet_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("socket failed");
            break;
        }

        // Forcefully attaching socket to the port 8080
        if (setsockopt(inet_server_fd, SOL_SOCKET,
                       SO_REUSEADDR, &inet_opt,
                       sizeof(inet_opt)))
        {
            perror("setsockopt");
            break;
        }
        inet_address.sin_family = AF_INET;
        inet_address.sin_addr.s_addr = INADDR_ANY;
        inet_address.sin_port = htons(PORT);

        // Forcefully attaching socket to the port 8080
        if (bind(inet_server_fd, (struct sockaddr *)&inet_address,
                 sizeof(inet_address)) < 0)
        {
            perror("bind failed");
            break;
        }
        if (listen(inet_server_fd, 3) < 0)
        {
            perror("listen");
            break;
        }

        printf("[Info] Inet socket : waiting for conn req\n");

        // Try until success
        while (inet_new_socket <= 0)
        {
            inet_new_socket = accept(inet_server_fd, (struct sockaddr *)&inet_address,
                                     &inet_addrlen);
            if (inet_new_socket < 0)
            {
                perror("accept() failed");
            }
        }

        // Set sd to non-blocking
        int flags1 = fcntl(sd, F_GETFL, 0);
        fcntl(sd, F_SETFL, flags1 | O_NONBLOCK);

        // Set sd2 to non-blocking
        int flags2 = fcntl(sd2, F_GETFL, 0);
        fcntl(sd2, F_SETFL, flags2 | O_NONBLOCK);

        // Set inet_server_fd to non-blocking
        int flags3 = fcntl(inet_server_fd, F_GETFL, 0);
        fcntl(inet_server_fd, F_SETFL, flags3 | O_NONBLOCK);

        // Set inet_new_socket to non-blocking
        int flags4 = fcntl(inet_new_socket, F_GETFL, 0);
        fcntl(inet_new_socket, F_SETFL, flags4 | O_NONBLOCK);

        printf("[Info] Inet socket : client connected\n");

        while (TRUE)
        {
            ssize_t unix_recv_len;
            unix_recv_len = recv(sd2, buffer, sizeof(buffer), 0);
            if (unix_recv_len > 0)
            {
                printf("[ME] : %s", buffer);

                if (inet_new_socket != -1 && inet_server_fd != -1)
                {
                    ssize_t inet_send_len;

                    inet_send_len = send(inet_new_socket, buffer, sizeof(buffer), 0);
                    if (inet_send_len > 0)
                    {
                        // Data was sent successfully
                    }
                    else if (inet_send_len == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    {
                        // No data was sent (not an error)
                    }
                    else
                    {
                        // Handle other errors
                        perror("send() failed");
                        break;
                    }
                }
                // Close the connection if the user has typed 'quit'
                if (strcmp(buffer, "quit\n") == 0)
                {
                    if (inet_new_socket != -1 && inet_server_fd != -1)
                    {
                        printf("[Server] quit\n");

                        // closing the connected socket
                        close(inet_new_socket);
                        // closing the listening socket
                        close(inet_server_fd);
                    }

                    inet_new_socket = -1;
                    inet_server_fd = -1;

                    printf("[Info] Closing sockets\n");

                    break;
                }
            }
            else if (unix_recv_len == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
            {
                // No data was available to read (not an error)
            }
            else
            {
                // Handle other errors
                perror("recv() failed");
                break;
            }

            if (inet_new_socket != -1 && inet_server_fd != -1)
            {
                ssize_t inet_recv_len;
                inet_recv_len = recv(inet_new_socket, inet_buffer, sizeof(inet_buffer), 0);
                if (inet_recv_len > 0)
                {
                    printf("[YOU] : %s", inet_buffer);
                    // Close the connection if the inet_client has typed 'quit'
                    if (strcmp(inet_buffer, "quit\n") == 0)
                    {
                        // closing the connected socket
                        close(inet_new_socket);
                        // closing the listening socket
                        close(inet_server_fd);

                        inet_new_socket = -1;
                        inet_server_fd = -1;

                        printf("[Info] Closing sockets\n");
                    }
                }
                else if (inet_recv_len == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                {
                    // No data was available to read (not an error)
                }
                else
                {
                    // Handle other errors
                    perror("recv() failed");
                    break;
                }
            }
        }

    } while (FALSE);

    rc = send(sd2, ": Success\n", strlen(": Success\n"), 0);
    if (rc < 0)
    {
        perror("recv() failed");
    }

    if (sd != -1)
        close(sd);

    if (sd2 != -1)
        close(sd2);

    unlink(SERVER_PATH);

    return 0;
}