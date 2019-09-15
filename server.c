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

// Constants
#define PORT_NUMBER 12345
#define MAX_CONNECTIONS 5

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
 *  Main method
 */
int main(int argc, char *argv[])
{
    // Varibales declaration
    int socket_fd; // Socket file descriptors
    int new_socket_fd;// Socket file descriptors
    int port_no;
    socklen_t client_len; // Client address
    struct sockaddr_in server_addr, client_addr;
    int num;
    char buffer[256];

    // Creating new socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0){
        error("Error in opening the socket");
    }
    // Setting all zeros to the buffer
    bzero((char *) &server_addr, sizeof(server_addr));

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
    if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        error("Error in binding the socket.");
    }

    // Listening to connections     
    listen(socket_fd, MAX_CONNECTIONS);

    // Accepting clients
    client_len = sizeof(client_addr);
    new_socket_fd = accept(socket_fd,  (struct sockaddr *) &client_addr, &client_len);
    
    if (new_socket_fd < 0){
        error("Error on accepting client");
    }
        
    bzero(buffer, 256);

    num = read(new_socket_fd, buffer, 255);
    if (num < 0){
        error("Error in reading from socket");
    }
    printf("Here is the message: %s\n", buffer);

    num = write(new_socket_fd, "I got your message", 18);
    if (num < 0){
        error("Error in writing to socket");
    }


    close(new_socket_fd);
    close(socket_fd);
    return 0;
}

