/**************************************************************************/
/* This sample program provides code for a client application that uses     */
/* AF_UNIX address family                                                 */
/**************************************************************************/
/**************************************************************************/
/* Header files needed for this sample program                            */
/**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h> // Include for close()

/**************************************************************************/
/* Constants used by this program                                         */
/**************************************************************************/
#define SERVER_PATH "./sock_addr"
#define BUFFER_LENGTH 250
#define TRUE 1
#define FALSE 0

/* Pass in 1 parameter which is either the */
/* path name of the server as a UNICODE    */
/* string, or set the server path in the   */
/* #define SERVER_PATH which is a CCSID    */
/* 500 string.                             */
int main(int argc, char *argv[])
{
    /***********************************************************************/
    /* Variable and structure definitions.                                 */
    /***********************************************************************/
    int sd = -1, rc, bytesReceived;
    char buffer[BUFFER_LENGTH];
    struct sockaddr_un serveraddr;

    /***********************************************************************/
    /* A do/while(FALSE) loop is used to make error cleanup easier.  The   */
    /* close() of the socket descriptor is only done once at the very end  */
    /* of the program.                                                     */
    /***********************************************************************/
    do
    {
        /********************************************************************/
        /* The socket() function returns a socket descriptor, which represents   */
        /* an endpoint.  The statement also identifies that the UNIX  */
        /* address family with the stream transport (SOCK_STREAM) will be   */
        /* used for this socket.                                            */
        /********************************************************************/
        sd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sd < 0)
        {
            perror("socket() failed");
            break;
        }

        /********************************************************************/
        /* If an argument was passed in, use this as the server, otherwise  */
        /* use the #define that is located at the top of this program.      */
        /********************************************************************/
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sun_family = AF_UNIX;
        if (argc > 1)
            strcpy(serveraddr.sun_path, argv[1]);
        else
            strcpy(serveraddr.sun_path, SERVER_PATH);

        /********************************************************************/
        /* Use the connect() function to establish a connection to the      */
        /* server.                                                          */
        /********************************************************************/
        rc = connect(sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
        if (rc < 0)
        {
            perror("connect() failed");
            break;
        }

        /********************************************************************/
        /* Get User to specify the data to be sent to the server.           */
        /* Use the send() function to send the data to the server.          */
        /********************************************************************/
        while (TRUE)
        {
            printf("Enter message to send: ");
            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);

            rc = send(sd, buffer, sizeof(buffer), 0);
            if (rc < 0)
            {
                perror("send() failed");
                break;
            }

            // Close the connection if the user has typed '\quit'
            if (strcmp(buffer, "\\quit\n") == 0)
                break;
        }

    } while (FALSE);

    /***********************************************************************/
    /* Close down any open socket descriptors                              */
    /***********************************************************************/
    if (sd != -1)
        close(sd);

    return 0;
}