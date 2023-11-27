#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h> // Include for close()

#define SERVER_PATH "./local_server2"
#define BUFFER_LENGTH 250
#define TRUE 1
#define FALSE 0

int main()
{
    int sd = -1, rc, bytesReceived;
    char buffer[BUFFER_LENGTH];
    struct sockaddr_un serveraddr;

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

        rc = connect(sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
        if (rc < 0)
        {
            perror("connect() failed");
            break;
        }

        printf("[Info] Unix Socket : connected to the server\n");

        /********************************************************************/
        /* Get User to specify the data to be sent to the server.           */
        /* Use the send() function to send the data to the server.          */
        /********************************************************************/
        while (TRUE)
        {
            printf("> Enter: ");
            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);

            rc = send(sd, buffer, sizeof(buffer), 0);
            if (rc < 0)
            {
                perror("send() failed");
                break;
            }

            if (strcmp(buffer, "quit\n") == 0)
            {
                printf("Terminate...\n");
                break;
            }
        }

    } while (FALSE);

    rc = recv(sd, buffer, sizeof(buffer), 0);
    if (rc < 0)
    {
        perror("send() failed");
    }

    if (strcmp(buffer, ": Success\n") == 0)
    {
        printf("%s", buffer);
        printf("[Info] Closing socket\n");
    }

    if (sd != -1)
        close(sd);

    return 0;
}