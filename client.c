/*
* Process Management and Distributed Computing
* Queensland University of Technology
* September 2019
*
* Client Implementation
* Ruchina Aidan Perera
*
*/

// Includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define MAX_BUFFER 255
#define SOCKET_ADDRESS struct sockaddr

// Global variables
int socket_server, socket_client;

/**
 * Exit function 
 */
void signal_callback_handler(int signum)
{
    printf("\nDisconnecting client from server.\n");
    close(socket_client);

    printf("Client disconnected.\n");
    // Exit program
    exit(signum);
}

/**
 * Chat function
 * Will handle the server communication 
*/
void chat()
{
    char buff[MAX_BUFFER];
    int n;

    memset(buff, 0, sizeof(buff));
    read(socket_server, buff, sizeof(buff));
    printf("%s", buff);

    // Looping till the client exits
    for (;;)
    {
        // Setting the buffer all zeros
        memset(buff, 0, sizeof(buff));

        n = 0;
        // Getting input from the client
        while ((buff[n++] = getchar()) != '\n')
            ;
      
        if (strncmp("BYE", buff, 3) == 0)
        {
            printf("Disconnecting client from server.\n");
            close(socket_client);

            printf("Client disconnected.\n");
            exit(0);
        }

        write(socket_server, buff, sizeof(buff));

        // Setting the buffer all zeros
        memset(buff, 0, sizeof(buff));

        // Reading from server
        read(socket_server, buff, sizeof(buff));
        printf("%s", buff);

        // Detect SIGINT
        signal(SIGINT, signal_callback_handler);
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in servaddr, cli;
    struct hostent *server;

    // Creating socket
    socket_server = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_server == -1)
    {
        printf("Client Error: socket creation failed.\n");
        exit(-1);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Server configuration
    if (argc < 3)
    {
        fprintf(stderr, "Error: usage %s hostname port.\n", argv[0]);
        exit(-1);
    }
    else
    {
        if (argv[1] != NULL && argv[2] != NULL)
        {
           server = gethostbyname(argv[1]); 
            servaddr.sin_family = AF_INET;
            memcpy((char *)server->h_addr_list[0], (char *)&servaddr.sin_addr.s_addr, server->h_length);
            //servaddr.sin_addr.s_addr = gethostbyname(argv[1])->h_addr_list[0];
            servaddr.sin_port = htons(atoi(argv[2]));
        }
        else
        {
            fprintf(stderr, "Client Error: Error identifying host.\n");
            exit(-1);
        }
    }

    // Client server connection
    if (connect(socket_server, (SOCKET_ADDRESS *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("Client Error: Connection failed.\n");
        exit(-1);
    }

    // Detect SIGINT
    signal(SIGINT, signal_callback_handler);

    // Function for chatting between client and server
    chat();

    // After chatting closing the socket
    close(socket_server);
}