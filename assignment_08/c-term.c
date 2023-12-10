#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h> // Include for close()
#include <fcntl.h>  // Include for fcntl()
#include <signal.h>
#include <errno.h> // Include for errno

#define BUFFER_LENGTH 250
#define TRUE 1
#define FALSE 0

void handler(int sig);

int unix_client_sd = -1;
struct sockaddr_un serveraddr;

int main()
{
    // Signal handler
    signal(SIGINT, handler);

    char buffer[BUFFER_LENGTH];
    char recv_buffer[BUFFER_LENGTH];

    if ((unix_client_sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() failed");
        exit(0);
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;

#ifdef C1
    strcpy(serveraddr.sun_path, "./local_server1");
#elif C2
    strcpy(serveraddr.sun_path, "./local_server2");
#elif C3
    strcpy(serveraddr.sun_path, "./local_server3");
#elif C4
    strcpy(serveraddr.sun_path, "./local_server4");
#endif

    if (connect(unix_client_sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr)) < 0)
    {
        perror("connect() failed");
        exit(0);
    }

    printf("[Info] Unix Socket : connected to the server\n");

    // Set non-blocking
    int flags = fcntl(unix_client_sd, F_GETFL, 0);
    fcntl(unix_client_sd, F_SETFL, flags | O_NONBLOCK);

    while (TRUE)
    {
        printf("> Enter: ");
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);

        if (send(unix_client_sd, buffer, sizeof(buffer), 0) < 0)
        {
            perror("send() failed");
            break;
        }

        usleep(100000); // Wait for 100 milliseconds

        // Receive from unix server
        memset(recv_buffer, 0, sizeof(recv_buffer));
        ssize_t recv_len = recv(unix_client_sd, recv_buffer, BUFFER_LENGTH, 0);
        if (recv_len == -1)
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
        else
        {
            printf("Debug: %s", recv_buffer);
            if (strcmp(recv_buffer, "Disconnect\n") == 0)
            {
                printf("Terminate...\n");
                break;
            }
        }
    }

    if (unix_client_sd != -1)
        close(unix_client_sd);

    return 0;
}

void handler(int sig)
{
    (void)sig;
    printf("(시그널 핸들러) 마무리 작업 시작!\n");
    if (unix_client_sd != -1)
        close(unix_client_sd);
    // Force exit
    exit(EXIT_SUCCESS);
}
