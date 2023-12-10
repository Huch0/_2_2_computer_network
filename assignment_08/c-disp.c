#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>     // Include for close()
#include <netinet/in.h> // Include for INET sockets
#include <fcntl.h>      // Include for fcntl()
#include <arpa/inet.h>  // Include for inet_pton()
#include <errno.h>      // Include for errno
#include <signal.h>
#include <stdlib.h>

#define BUFFER_LENGTH 250
#define TRUE 1
#define FALSE 0
#define PORT 8080 // Port number for INET socket

int setup_unix_server(int *unix_acceptor_sd, struct sockaddr_un *serveraddr, int *unix_server_sd);
int setup_inet_client(int *inet_client_fd, struct sockaddr_in *inet_serv_addr);
void handle_unix_inet(int unix_server_sd, int inet_client_fd);
void handler(int sig);

// UNIX server
int unix_acceptor_sd = -1, unix_server_sd = -1;
struct sockaddr_un serveraddr;

// INET Client
int inet_client_fd = -1;
struct sockaddr_in inet_serv_addr;

char server_path[40];
int main()
{
    // Signal handler
    signal(SIGINT, handler);

    if (setup_unix_server(&unix_acceptor_sd, &serveraddr, &unix_server_sd) < 0)
    {
        perror("setup_unix_server() failed");
        return -1;
    }

    if (setup_inet_client(&inet_client_fd, &inet_serv_addr) < 0)
    {
        perror("setup_inet_client() failed");
        return -1;
    }

    handle_unix_inet(unix_server_sd, inet_client_fd);

    if (unix_acceptor_sd != -1)
        close(unix_acceptor_sd);

    if (unix_server_sd != -1)
        close(unix_server_sd);

    if (inet_client_fd != -1)
        close(inet_client_fd);

    unlink(server_path);

    return 0;
}

int setup_unix_server(int *unix_acceptor_sd, struct sockaddr_un *serveraddr, int *unix_server_sd)
{
    if ((*unix_acceptor_sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() failed");
        return -1;
    }

    memset(serveraddr, 0, sizeof(*serveraddr));
    serveraddr->sun_family = AF_UNIX;

#ifdef C1
    strcpy(server_path, "./local_server1");
#elif C2
    strcpy(server_path, "./local_server2");
#elif C3
    strcpy(server_path, "./local_server3");
#elif C4
    strcpy(server_path, "./local_server4");
#endif

    strcpy(serveraddr->sun_path, server_path);

    if (bind(*unix_acceptor_sd, (struct sockaddr *)serveraddr, SUN_LEN(serveraddr)) < 0)
    {
        perror("bind() failed");
        return -1;
    }

    if (listen(*unix_acceptor_sd, 10) < 0)
    {
        perror("listen() failed");
        return -1;
    }

    printf("[Info] Unix socket : waiting for conn req\n");
    socklen_t addrlen = sizeof(*serveraddr);
    if ((*unix_server_sd = accept(*unix_acceptor_sd, (struct sockaddr *)serveraddr, &addrlen)) < 0)
    {
        perror("accept() failed");
        return -1;
    }

    printf("[Info] Unix socket : client connected\n");

    // Set non-blocking
    int flags = fcntl(*unix_server_sd, F_GETFL, 0);
    fcntl(*unix_server_sd, F_SETFL, flags | O_NONBLOCK);

    return 1;
}

int setup_inet_client(int *inet_client_fd, struct sockaddr_in *inet_serv_addr)
{
    if ((*inet_client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() failed");
        return -1;
    }

    inet_serv_addr->sin_family = AF_INET;
    inet_serv_addr->sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &(inet_serv_addr->sin_addr)) <= 0)
    {
        perror("inet_pton() failed");
        return -1;
    }

    if (connect(*inet_client_fd, (struct sockaddr *)inet_serv_addr, sizeof(*inet_serv_addr)) < 0)
    {
        perror("connect() failed");
        return -1;
    }
    // Set non-blocking
    int flags = fcntl(*inet_client_fd, F_GETFL, 0);
    if (fcntl(*inet_client_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl() failed");
        return -1;
    }
    printf("[Info] Inet Socket : connected to the server\n");

    return 1;
}
void handle_unix_inet(int unix_server_sd, int inet_client_fd)
{
    char buffer[BUFFER_LENGTH];

    while (TRUE)
    {
        // INET -> UNIX
        memset(buffer, 0, BUFFER_LENGTH);
        ssize_t bytes_received = recv(inet_client_fd, buffer, BUFFER_LENGTH, 0);
        if (bytes_received == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // No data to read
            }
            else
            {
                perror("recv() from INET failed");
                break;
            }
        }
        else if (bytes_received == 0)
        {
        }
        else
        {
            printf("%s", buffer);

            if (strcmp(buffer, "Disconnect\n") == 0)
            {
                printf("Terminate...\n");

                // Send the received message to the UNIX client
                // try until success
                while (send(unix_server_sd, buffer, bytes_received, 0) < 0)
                {
                    // handle non-blocking
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
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
                break;
            }
        }

        // UNIX -> INET
        memset(buffer, 0, BUFFER_LENGTH);
        ssize_t bytes_received_unix = recv(unix_server_sd, buffer, BUFFER_LENGTH, 0);
        if (bytes_received_unix == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // No data to read
            }
            else
            {
                perror("recv() from UNIX failed");
                break;
            }
        }
        else if (bytes_received_unix == 0)
        {
        }
        else
        {
            printf("[ME] : %s", buffer);

            if (send(inet_client_fd, buffer, bytes_received_unix, 0) < 0)
            {
                perror("send() failed");
                break;
            }
        }
    }
}

void handler(int sig)
{
    (void)sig; // unused
    printf("(시그널 핸들러) 마무리 작업 시작!\n");
    // close sockets
    // unlink socket file
    if (unix_acceptor_sd != -1)
        close(unix_acceptor_sd);

    if (unix_server_sd != -1)
        close(unix_server_sd);

    if (inet_client_fd != -1)
        close(inet_client_fd);

    unlink(server_path);
    // Force exit
    exit(EXIT_SUCCESS);
}