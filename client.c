/*
* Process Management and Distributed Computing
* Queensland University of Technology
* September 2019
*
* Client Implementation
* Ruchina Aidan Perera
*
*/

#include "client.h"

/**
 * Exit function 
 */
void signalCallbackHandler(int signum)
{
    if (is_inifite_loop == true)
    {
        printf("\n");
        is_inifite_loop = false;
        write(socket_server, "OK", 2);
    }
    else
    {
        write(socket_server, "BYE", 3); // Sending the server the exit message
        printf("\nDisconnecting client from server.\n");
        close(socket_client);

        printf("Client disconnected.\n");
        // Exit program
        exit(signum);
    }
}

/**
 * Chat function
 * Will handle the server communication 
*/
void chat()
{
    char buff[MAX_BUFFER];
    int n;
    char *value;
    char command[4];
    bool is_loop = false;

    memset(buff, 0, sizeof(buff));
    read(socket_server, buff, sizeof(buff));
    printf("%s", buff);

    // Looping till the client exits
    for (;;)
    {
        // Setting the buffer all zeros
        memset(buff, 0, MAX_BUFFER);

        // Disabling the input for the livefeed
        if (is_inifite_loop == false)
        {
            n = 0;
            // Getting input from the client
            while ((buff[n++] = getchar()) != '\n')
                ;
        }

        if (strncmp("BYE", buff, 3) == 0) // BYE Command
        {
            write(socket_server, buff, sizeof(buff));
            printf("Disconnecting client from server.\n");
            close(socket_client);

            printf("Client disconnected.\n");
            exit(0);
        }
        // else if (strncmp("LIVEFEED", buff, 8) == 0) // LIVEFEED Command
        // {

        //     write(socket_server, buff, sizeof(buff));
        //     // Setting the buffer all zeros
        //     memset(buff, 0, MAX_BUFFER);
        //     read(socket_server, buff, MAX_BUFFER);

        // }

        if (is_inifite_loop == false)
        {
            write(socket_server, buff, sizeof(buff));
        }

        // Setting the buffer all zeros
        memset(buff, 0, MAX_BUFFER);
        // Reading from server
        read(socket_server, buff, MAX_BUFFER);
        if (strncmp("OK", buff, 2) == 0)
        {
            is_inifite_loop = true;
            continue;
        }
        else
        {
            // Printing loop lines
            value = strtok(buff, "|LL|");
            while (value != NULL)
            {
                printf("%s", value);
                value = strtok(NULL, "|LL|");
            }
        }

        // Detect SIGINT
        signal(SIGINT, signalCallbackHandler);
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in servaddr;
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
    signal(SIGINT, signalCallbackHandler);

    // Function for chatting between client and server
    chat();

    // After chatting closing the socket
    close(socket_server);
}