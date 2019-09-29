/*
* Process Management and Distributed Computing
* Queensland University of Technology
* September 2019
*
* Server Implementation
* Ruchina Aidan Perera
*
*/

// Includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_BUFFER 255
#define DEFAULT_PORT 12345
#define MAX_CLIENT_QUEUE 5
#define SOCKET_ADDRESS struct sockaddr

// Global variables
int socket_server;
struct Client
{
    int client_id;
    uint8_t channel_id;
};

/**
 * Exit function 
 */
void signal_callback_handler(int signum)
{
    printf("\nClosing server.\n");
    close(socket_server);

    printf("Server closed.\n");
    // Exit program
    exit(signum);
}
/**
 * Client command check
 * Return:
 *  0 : All good
 *  -1 : Error
 *  1 : Continue the loop
 *  2 : Break the loop
 *   
 */
int checkClientCommand(struct Client cl, char *buffer, char *error_message)
{
    // Setting error message all zeros
    memset(error_message, 0, MAX_BUFFER);
    if (strncmp("BYE", buffer, 3) == 0) // BYE command
    {
        close(cl.client_id);
        return 1;
    }
    else if (strncmp("SUB", buffer, 3) == 0) // SUB command
    {
        char *value;
        value = strtok(buffer, " "); 
        value = strtok(NULL, " ");
        int channel_id = atoi(value);
        if( channel_id > 0 && channel_id < 256){
            memset(buffer, 0, MAX_BUFFER);
            sprintf(buffer, "Subscribed  to  channel %d.\n", channel_id);
            return 1;
        }else{
            sprintf(error_message, "Invalid  channel: %d.\n", channel_id);
            return -1;
        }
    }
}

/**
 * Chat function
 * Will handle the client communication 
*/
void chat(struct Client cl)
{
    char buff[MAX_BUFFER];
    char error_message[MAX_BUFFER];
    int n;
    int socket_client = cl.client_id;

    // Sending the welcome message to client
    memset(buff, 0, MAX_BUFFER);
    sprintf(buff, "Welcome! Your client ID is %d.\n", socket_client);
    write(socket_client, buff, sizeof(buff));

    // Looping till the server exits
    for (;;)
    {
        // Setting the buffer all zeros
        memset(buff, 0, MAX_BUFFER);

        // Reading the message from the client
        read(socket_client, buff, sizeof(buff));

        // Client command / input check
        int response = checkClientCommand(cl, buff, error_message);
        if (response == 1)
        {
            write(socket_client, buff, sizeof(buff));
            continue;
        }
        else if (response == 2)
        {
            break;
        }
        else if (response == -1)
        {
            // Write to client
            write(socket_client, error_message, sizeof(error_message));
            continue;
        }

        // print buffer which contains the client contents
        memset(buff, 0, MAX_BUFFER);
        n = 0;
        // copy server message in the buffer
        while ((buff[n++] = getchar()) != '\n')
            ;

        // Write to client
        write(socket_client, buff, sizeof(buff));

        // Detect SIGINT
        signal(SIGINT, signal_callback_handler);
    }
}

// Driver function
int main(int argc, char *argv[])
{
    int socket_client, len;
    struct sockaddr_in servaddr, cli;

    // Creating socket
    socket_server = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_server == -1)
    {
        printf("Server Error: socket creation failed.\n");
        exit(-1);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Server configuration
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Retrieving port number from args
    if (argc < 2)
    {
        servaddr.sin_port = htons(DEFAULT_PORT);
    }
    else
    {
        servaddr.sin_port = htons(atoi(argv[1]));
    }
    // Binding server
    if ((bind(socket_server, (SOCKET_ADDRESS *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("Server Error: Socket bind failed.\n");
        exit(-1);
    }

    // Detect SIGINT
    signal(SIGINT, signal_callback_handler);

    // Listening to new clients
    if ((listen(socket_server, MAX_CLIENT_QUEUE)) != 0)
    {
        printf("Server Error: Server listening failed\n");
        exit(-1);
    }

    len = sizeof(cli);

    // Accepting new clients
    socket_client = accept(socket_server, (SOCKET_ADDRESS *)&cli, &len);
    if (socket_client < 0)
    {
        printf("Server Error: Server accepting failed.\n");
        exit(-1);
    }

    // Setting new client
    struct Client cl = {
        socket_client,
        0b00000000};

    // Function for chatting between client and server
    chat(cl);

    // After chatting closing the socket
    close(socket_server);
}