#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>
#include <fcntl.h> // Include for fcntl()
#include <errno.h> // Include for errno
#include <ctype.h>

#define BUFFER_LENGTH 250
#define TRUE 1
#define FALSE 0
#define PORT 8080 // Port number for INET socket

#define NUM_ROOMS 3 // Number of threads
#define NUM_CLIENTS 15

struct ChatRoom
{
    int room_id;
    char room_name[20];
    int sockets[5]; // 5 clients per room
    int count;
} typedef ChatRoom;

struct UserSockets
{
    int sockets[NUM_CLIENTS];
    int count;
} typedef UserSockets;

struct ChatInfo
{
    struct ChatRoom room;
    struct UserSockets new_users;
    struct UserSockets returned_users;
} typedef ChatInfo;

void handler(int signal);
void *thread_func(void *thread_id);
void setup_inetserver();
void handle_select(UserSockets waiting_users, ChatInfo chat_info[]);
int maxArr(int arr[], int n);
void send_menu(int socket);
void remove_from_sockets(int socket, int sockets[], int *count);
void add_to_sockets(int socket, int sockets[], int *count);

int access_sock = -1;
int comm_sock[NUM_CLIENTS] = {
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1};
int current_clients = 0;

volatile sig_atomic_t exit_flag = 0;
pthread_t threads[NUM_ROOMS];

ChatInfo chat_info[NUM_ROOMS] = {
    {.room.room_id = 0, .room.room_name = "Chatroom-0", .room.sockets = {-1, -1, -1, -1, -1}, .new_users.sockets = {-1, -1, -1, -1, -1}, .returned_users.sockets = {-1, -1, -1, -1, -1}},
    {.room.room_id = 1, .room.room_name = "Chatroom-1", .room.sockets = {-1, -1, -1, -1, -1}, .new_users.sockets = {-1, -1, -1, -1, -1}, .returned_users.sockets = {-1, -1, -1, -1, -1}},
    {.room.room_id = 2, .room.room_name = "Chatroom-2", .room.sockets = {-1, -1, -1, -1, -1}, .new_users.sockets = {-1, -1, -1, -1, -1}, .returned_users.sockets = {-1, -1, -1, -1, -1}},
};

// Initialize waiting_users with -1
UserSockets waiting_users = {.sockets = {-1, -1, -1, -1, -1}, .count = 0};
int main()
{
    signal(SIGINT, handler);

    int rc;
    long t;

    // Create threads
    for (t = 0; t < NUM_ROOMS; t++)
    {
        printf("[MAIN] Creating Room #%ld\n", t);
        rc = pthread_create(&threads[t], NULL, thread_func, (void *)&chat_info[t]);
        if (rc)
        {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }

    setup_inetserver();

    while (TRUE)
    {
        // 1. Call accept() to receive incoming connections
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(access_sock, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        // Check if there is any incoming connection
        // Max number of clients is 15
        if (current_clients < 15 && select(access_sock + 1, &readfds, NULL, NULL, &timeout) > 0)
        {
            printf("after select\n");
            if (FD_ISSET(access_sock, &readfds))
            {
                // Accept the connection
                int new_client = accept(access_sock, NULL, NULL);
                printf("DEBUG: new_client : %d\n", new_client);
                if (new_client == -1)
                {
                    if (errno == EINTR)
                    {
                        // Handle interrupted system call (e.g., retry accept)
                        printf("Interrupted system call\n");
                    }
                    else if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        // No incoming connection
                    }
                    else
                    {
                        perror("accept");
                        // Handle other errors
                        break;
                    }
                }
                else
                {
                    // Set non-blocking
                    int flags = fcntl(new_client, F_GETFL, 0);
                    fcntl(new_client, F_SETFL, flags | O_NONBLOCK);

                    // Add to comm_sock
                    add_to_sockets(new_client, comm_sock, &current_clients);

                    // Handle the new connection
                    printf("새로운 사용자가 접속했습니다 : %d\n", new_client);

                    // Send menu to new user
                    send_menu(new_client);

                    // Add to waiting_users
                    waiting_users.sockets[waiting_users.count] = new_client;
                    waiting_users.count++;
                }
            }
        }

        // 2. Check if there is any returned user
        // If there is, add them to waiting_users
        for (int i = 0; i < NUM_ROOMS; i++)
        {
            for (int j = 0; j < chat_info[i].returned_users.count; j++)
            {
                waiting_users.sockets[waiting_users.count] = chat_info[i].returned_users.sockets[j];
                chat_info[i].returned_users.sockets[j] = -1;
                waiting_users.count++;
            }
            chat_info[i].returned_users.count = 0;
        }

        // 3. Call select() to check if there is any message from any user
        handle_select(waiting_users, chat_info);
    }

    // Wait for all threads to complete
    for (t = 0; t < NUM_ROOMS; t++)
    {
        pthread_join(threads[t], NULL);
    }

    printf("[MAIN] All threads are done!\n");
    pthread_exit(NULL);

    return 0;
}

void *thread_func(void *thread_chat_info)
{
    ChatInfo *chat_info = (ChatInfo *)thread_chat_info;

    fd_set readfds;
    int ret;

    while (!exit_flag)
    {
        // printf("DEBUG: Thread #%d\n", chat_info->room.room_id);

        // 1. Check for new users
        if (chat_info->new_users.count > 0)
        {
            // Add new users to the room
            for (int i = 0; i < chat_info->new_users.count; i++)
            {
                chat_info->room.sockets[chat_info->room.count] = chat_info->new_users.sockets[i];
                chat_info->room.count++;

                // Log
                printf("[ch.%d] 새로운 참가자 : %d\n", chat_info->room.room_id, chat_info->new_users.sockets[i]);

                // Remove from new_users sockets
                chat_info->new_users.sockets[i] = -1;
            }
            chat_info->new_users.count = 0;
        }

        // 2. Check is there any message from any user
        // If there is, send it to all users in the room
        FD_ZERO(&readfds);
        for (int i = 0; i < chat_info->room.count; i++)
            FD_SET(chat_info->room.sockets[i], &readfds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        ret = select(maxArr(chat_info->room.sockets, 5) + 1, &readfds, NULL, NULL, &timeout);
        // printf("After select\n");

        int i = 0;

        switch (ret)
        {
        case -1:
            perror("select");
            break;
        case 0:
            // printf("select returns: 0\n");
            break;
        default:
            while (ret > 0)
            {
                if (FD_ISSET(chat_info->room.sockets[i], &readfds))
                {
                    ret--;
                    int selected_socket = chat_info->room.sockets[i];

                    char client_msg[BUFFER_LENGTH];
                    if (read(selected_socket, client_msg, BUFFER_LENGTH) < 0)
                    {
                        perror("read");
                        exit(1);
                    }
                    printf("[ch.%d] 사용자 %d 메시지 : %s", chat_info->room.room_id, selected_socket, client_msg);

                    // Check if the user wants to quit
                    if (strcmp(client_msg, "quit\n") == 0)
                    {
                        // Remove the user from the room
                        remove_from_sockets(selected_socket, chat_info->room.sockets, &chat_info->room.count);

                        // Add the user to returned_users
                        chat_info->returned_users.sockets[chat_info->returned_users.count] = selected_socket;
                        chat_info->returned_users.count++;

                        // Log
                        printf("[ch.%d] 사용자 %d를 채팅방에서 제거합니다.\n", chat_info->room.room_id, selected_socket);

                        // Send message to the user
                        char msg[] = "채팅방을 나갑니다\n";
                        if (write(selected_socket, msg, strlen(msg)) < 0)
                        {
                            perror("write");
                            exit(1);
                        }

                        continue;
                    }

                    // Check if the user is alone in the room
                    if (chat_info->room.count == 1)
                    {
                        printf("[ch.%d] 사용자 %d가 혼자여서 메시지를 전달안합니다.\n", chat_info->room.room_id, selected_socket);
                        continue;
                    }

                    char chat_msg[BUFFER_LENGTH];
                    sprintf(chat_msg, "[%d] %s", selected_socket, client_msg);

                    // Send message to all users in the room
                    for (int j = 0; j < chat_info->room.count; j++)
                    {
                        if (j != i && chat_info->room.sockets[j] != -1)
                        {
                            if (write(chat_info->room.sockets[j], chat_msg, strlen(chat_msg)) < 0)
                            {
                                perror("write");
                                exit(1);
                            }
                        }
                    }
                }
                i++;
            }
        }
    }

    pthread_exit(NULL);
}

void handler(int sig)
{
    signal(sig, SIG_IGN); // Ignore the signal (prevent recursive signal calls

    printf("(시그널 핸들러) 마무리 작업 시작!\n");

    // Set exit flag
    exit_flag = 1;

    // Wait for all threads to complete
    for (int i = 0; i < NUM_ROOMS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // close sockets
    if (access_sock != -1)
        close(access_sock);

    for (int i = 0; i < NUM_CLIENTS; i++)
    {
        if (comm_sock[i] != -1)
            close(comm_sock[i]);
    }
    // Force exit
    exit(EXIT_SUCCESS);
}

void setup_inetserver()
{
    struct sockaddr_in addr;

    if ((access_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    // Set socket option
    int reuse = 1;
    if (setsockopt(access_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        perror("setsockopt");
        exit(1);
    }

    if (bind(access_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(access_sock, 5) < 0)
    {
        perror("listen");
        exit(1);
    }

    // Set non-blocking
    int flags = fcntl(access_sock, F_GETFL, 0);
    if (fcntl(access_sock, F_SETFL, flags | O_NONBLOCK) == -1)
        perror("fcntl");
}

void send_room_list(int socket, ChatInfo chat_info[]);
void join_room(int socket, char client_msg[], ChatInfo chat_info[]);
void disconnect_client(int socket);
void invalid_option(int socket);

void handle_select(UserSockets waiting_users, ChatInfo chat_info[])
{
    fd_set readfds;
    int ret;

    // Config fd_set
    FD_ZERO(&readfds);
    for (int i = 0; i < NUM_CLIENTS; i++)
        FD_SET(waiting_users.sockets[i], &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    ret = select(maxArr(waiting_users.sockets, waiting_users.count) + 1, &readfds, NULL, NULL, &timeout);

    int i = 0;

    switch (ret)
    {
    case -1:
        perror("select");
        break;
    case 0:
        // printf("select returns: 0\n");
        break;
    default:
        while (ret > 0)
        {
            if (FD_ISSET(waiting_users.sockets[i], &readfds))
            {
                ret--;
                char client_msg[BUFFER_LENGTH];
                memset(client_msg, 0, BUFFER_LENGTH);
                if (read(waiting_users.sockets[i], client_msg, BUFFER_LENGTH) < 0)
                {
                    perror("read");
                    exit(1);
                }
                printf("사용자 %d 메시지 : %s", waiting_users.sockets[i], client_msg);

                // Tokenize the input
                char *token = strtok(client_msg, " \n");
                if (token != NULL)
                {
                    char option_number = token[0];

                    switch (option_number)
                    {
                    case '0':
                        send_menu(waiting_users.sockets[i]);
                        break;
                    case '1':
                        send_room_list(waiting_users.sockets[i], chat_info);
                        break;
                    case '2':
                        join_room(waiting_users.sockets[i], client_msg, chat_info);
                        break;
                    case '3':
                        disconnect_client(waiting_users.sockets[i]);
                        break;
                    default:
                        invalid_option(waiting_users.sockets[i]);
                        break;
                    }
                }
            }
            i++;
        }
    }
}

int maxArr(int arr[], int n)
{
    int max = arr[0];
    for (int i = 1; i < n; i++)
        if (arr[i] > max)
            max = arr[i];
    return max;
}

char menu[] = "<MENU>\n1.채팅방 목록 보기\n2.채팅방 참여하기(사용법 : 2 <채팅방 번호>)\n3. 프로그램 종료\n(0을 입력하면 메뉴가 다시 표시됩니다)\n";
void send_menu(int socket)
{
    if (write(socket, menu, strlen(menu)) < 0)
    {
        perror("write");
        exit(1);
    }
}

void get_chatroom_info(ChatInfo chat_info[], char chatroom_info[]);

void send_room_list(int socket, ChatInfo chat_info[])
{
    char chatroom_info[BUFFER_LENGTH];
    // printf("Debug: %d\n", socket);
    get_chatroom_info(chat_info, chatroom_info);
    // printf("Debug: %s\n", chatroom_info);

    if (write(socket, chatroom_info, strlen(chatroom_info)) < 0)
    {
        perror("write");
        exit(1);
    }
}

void get_chatroom_info(ChatInfo chat_info[], char chatroom_info[])
{
    /*
    <ChatRoom info>
    [ID: 0] Chatroom-0 (1/5)
    [ID: 1] Chatroom-1 (2/5)
    [ID: 2] Chatroom-2 (0/5)
    */
    char temp[BUFFER_LENGTH];

    strcpy(chatroom_info, "<ChatRoom info>\n");

    for (int i = 0; i < NUM_ROOMS; i++)
    {
        sprintf(temp, "[ID: %d] %s (%d/5)\n", chat_info[i].room.room_id, chat_info[i].room.room_name, chat_info[i].room.count);
        strcat(chatroom_info, temp);
    }
}

void join_room(int socket, char client_msg[], ChatInfo chat_info[])
{
    /*
    client_msg = "2 0\n"
    */

    if (!isdigit(client_msg[2]))
    {
        invalid_option(socket);
        return;
    }

    int room_id = client_msg[2] - '0';

    // Check if the room is full
    if (chat_info[room_id].room.count == 5)
    {
        char msg[] = "채팅방이 가득찼습니다\n";
        if (write(socket, msg, strlen(msg)) < 0)
        {
            perror("write");
            exit(1);
        }
        return;
    }

    // Remove the user from waiting_users
    remove_from_sockets(socket, waiting_users.sockets, &waiting_users.count);

    // Add the user to the room
    chat_info[room_id].new_users.sockets[chat_info[room_id].new_users.count] = socket;
    chat_info[room_id].new_users.count++;

    // Log
    printf("[MAIN] 사용자 %d가 채팅방 %d에 참여합니다\n", socket, room_id);

    // Send message to the user
    char msg[BUFFER_LENGTH];
    sprintf(msg, "%s 방에 접속하였습니다.\n", chat_info[room_id].room.room_name);
    if (write(socket, msg, strlen(msg)) < 0)
    {
        perror("write");
        exit(1);
    }
}

void disconnect_client(int socket)
{
    char msg[] = "Disconnect\n";
    if (write(socket, msg, strlen(msg)) < 0)
    {
        perror("write");
        exit(1);
    }

    // Remove the user from waiting_users
    remove_from_sockets(socket, waiting_users.sockets, &waiting_users.count);

    // Remove the user from comm_sock
    remove_from_sockets(socket, comm_sock, &current_clients);

    if (socket != -1)
        close(socket);

    // Log
    printf("[MAIN] %d 사용자와의 접속을 해제합니다.\n", socket);
}

void invalid_option(int socket)
{
    char msg[] = "잘못된 입력입니다\n";
    if (write(socket, msg, strlen(msg)) < 0)
    {
        perror("write");
        exit(1);
    }
}

void remove_from_sockets(int socket, int sockets[], int *count)
{
    // Remove the socket from sockets
    // Shift the rest of the array to the left
    for (int i = 0; i < *count; i++)
    {
        if (sockets[i] == socket)
        {
            for (int j = i; j < *count - 1; j++)
            {
                sockets[j] = sockets[j + 1];
            }
            sockets[*count - 1] = -1;

            break;
        }
    }
    (*count)--;
}

void add_to_sockets(int socket, int sockets[], int *count)
{
    // Find the first empty slot
    for (int i = 0; i < *count; i++)
    {
        if (sockets[i] == -1)
        {
            sockets[i] = socket;
            break;
        }
    }
    (*count)++;
}