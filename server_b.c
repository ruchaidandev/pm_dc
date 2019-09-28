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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

// Constants
#define PORT_NUMBER 12345
#define MAX_CONNECTIONS 5

// GLobal variables declaration
int socket_fd;     // Socket file descriptors
int new_socket_fd; // Socket file descriptors
int channels[256];

/**
 * Error Method
 * Will print out the error
 */
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/**
 * Exit function 
 */
void signal_callback_handler(int signum)
{
    printf("\nclosing server.\n");
    close(new_socket_fd);
    close(socket_fd);

    printf("Server closed.\n");
    // Exit program
    exit(signum);
}

/**
 * Write message to client
 */
void write_to_client(int client_id, char *message)
{
    write(client_id, message, strlen(message));
}

/**
 *  Main method
 */
int main(int argc, char *argv[])
{
    // Varibales declaration

    int port_no;
    socklen_t client_len; // Client address
    struct sockaddr_in server_addr, client_addr;
    int num;
    char buffer[256];

    // Detect SIGINT
    signal(SIGINT, signal_callback_handler);

    // Creating new socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        error("Error in opening the socket.");
    }
    // Setting all zeros to the buffer
    // bzero((char *) &server_addr, sizeof(server_addr));
    memset(&server_addr, 0, sizeof(server_addr));

    // Retrieving the port number
    if (argc < 2)
    {
        port_no = PORT_NUMBER;
    }
    else
    {
        port_no = atoi(argv[1]);
    }

    // Setting up the address family
    server_addr.sin_family = AF_INET;
    // Setting the server address
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // Converting port to host byte order
    server_addr.sin_port = htons(port_no);

    // Binds the socket to the address
    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Error in binding the socket.");
    }

    while (1)
    {
        // Listening to connections
        listen(socket_fd, MAX_CONNECTIONS);

        // Accepting clients
        client_len = sizeof(client_addr);
        new_socket_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);

        // Welcome message

        memset(&buffer, 0, sizeof(buffer));

        char *message;
        sprintf(message, "Welcome! Your client ID is %d.", new_socket_fd);
        write_to_client(new_socket_fd, message);

        if (new_socket_fd < 0)
        {
            error("Error on accepting client.");
        }

        // bzero(buffer, 256);
        memset(&buffer, 0, sizeof(buffer));

        num = read(new_socket_fd, buffer, 255);
        if (num < 0)
        {
            error("Error in reading from socket.");
        }

        // Channel
        
        char *command;
        // for(int i=0; command = strtok(buffer," "), command != NULL; command = strtok(NULL, " "), i++){
        //     if (strncmp(command, "SUB", 3 ) == 0)
        //     {
        //         // char *output;
        //         // command = strtok(NULL, " ");
        //         // if (atoi(command) > 0 && atoi(command) < 256)
        //         // {
        //         //     sprintf(output, "Subscribed to channel %d.", atoi(command));
        //         //     write_to_client(new_socket_fd, output);
        //         // }
        //         // else
        //         // {
        //         //     sprintf(output, "Invalid channel: %d.", atoi(command));
        //         //     write_to_client(new_socket_fd, output);
        //         // }
        //         char *output;
        //         sprintf(output, "SUBBED.");
        //         write_to_client(new_socket_fd, output);
                
        //     }
        //     else if (strncmp(command, "BYE",3) == 0)
        //     {
        //         char *output;
        //         sprintf(output, "Disconnecting from server.");
        //         write_to_client(new_socket_fd, output);
        //     }
        // }
       
        memset(&buffer, 0, sizeof(buffer));

        num = write(new_socket_fd, "I got your message", 18);
        if (num < 0)
        {
            error("Error in writing to socket.");
        }
    }

    return 0;
}
