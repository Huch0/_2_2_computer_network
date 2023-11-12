/**************************************************************************/
/* This example program provides code for a server application that uses     */
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

int main()
{
    /***********************************************************************/
    /* Variable and structure definitions.                                 */
    /***********************************************************************/
    int sd = -1, sd2 = -1;
    int rc, length;
    char buffer[BUFFER_LENGTH];
    struct sockaddr_un serveraddr;

    struct sockaddr_un clientaddr;
    socklen_t clientaddr_len;

    /***********************************************************************/
    /* A do/while(FALSE) loop is used to make error cleanup easier.  The   */
    /* close() of each of the socket descriptors is only done once at the  */
    /* very end of the program.                                            */
    /***********************************************************************/
    do
    {
        /********************************************************************/
        /* The socket() function returns a socket descriptor, which represents   */
        /* an endpoint.  The statement also identifies that the UNIX        */
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
        /* After the socket descriptor is created, a bind() function gets a */
        /* unique name for the socket.                                      */
        /********************************************************************/
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sun_family = AF_UNIX;
        strcpy(serveraddr.sun_path, SERVER_PATH);

        rc = bind(sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
        if (rc < 0)
        {
            perror("bind() failed");
            break;
        }

        /********************************************************************/
        /* The listen() function allows the server to accept incoming       */
        /* client connections.  In this example, the backlog is set to 10.  */
        /* This means that the system will queue 10 incoming connection     */
        /* requests before the system starts rejecting the incoming         */
        /* requests.                                                        */
        /********************************************************************/
        rc = listen(sd, 10);
        if (rc < 0)
        {
            perror("listen() failed");
            break;
        }

        printf("Ready for client connect().\n");

        /********************************************************************/
        /* The server uses the accept() function to accept an incoming      */
        /* connection request.  The accept() call will block indefinitely   */
        /* waiting for the incoming connection to arrive.                   */
        /********************************************************************/

        // Try until success
        while (sd2 < 0)
        {
            sd2 = accept(sd, (struct sockaddr *)&clientaddr, &clientaddr_len);
            if (sd2 < 0)
            {
                perror("accept() failed");
            }
        }

        /********************************************************************/
        /* In this example we know that the client will send 250 bytes of   */
        /* data over.  Knowing this, we can use the SO_RCVLOWAT socket      */
        /* option and specify that we don't want our recv() to wake up      */
        /* until all 250 bytes of data have arrived.                        */
        /********************************************************************/
        length = BUFFER_LENGTH;
        rc = setsockopt(sd2, SOL_SOCKET, SO_RCVLOWAT,
                        (char *)&length, sizeof(length));
        if (rc < 0)
        {
            perror("setsockopt(SO_RCVLOWAT) failed");
            break;
        }
        /****************************************************/
        /* Receive that 250 bytes data from the client */
        /****************************************************/
        while (TRUE)
        {
            rc = recv(sd2, buffer, sizeof(buffer), 0);
            if (rc < 0)
            {
                perror("recv() failed");
                break;
            }
            if (rc == 0 ||
                rc < sizeof(buffer))
            {
                printf("The client closed the connection before all of the\n");
                printf("data was sent\n");
                break;
            }

            /********************************************************************/
            /* Print message received from client                                */
            /********************************************************************/
            printf("%s\n", buffer);

            // Close the connection if the user has typed '\quit'
            if (strcmp(buffer, "\\quit\n") == 0)
                break;
        }

        /********************************************************************/
        /* Program complete                                                 */
        /********************************************************************/

    } while (FALSE);

    /***********************************************************************/
    /* Close down any open socket descriptors                              */
    /***********************************************************************/
    if (sd != -1)
        close(sd);

    if (sd2 != -1)
        close(sd2);

    /***********************************************************************/
    /* Remove the UNIX path name from the file system                      */
    /***********************************************************************/
    unlink(SERVER_PATH);

    return 0;
}