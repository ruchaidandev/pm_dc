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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/**
 * Error Method
 * Will print out the error
 */
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

/**
 * Main method
 */
int main(int argc, char *argv[])
{
    // Variable initialisation
    int socket_fd; // file descriptors
    int port_no;
    int num;
    // Server address
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    // Checking arguments
    if (argc < 3)
    {
        fprintf(stderr, "Error: usage %s hostname port.\n", argv[0]);
        exit(0);
    }

    // Connecting to socket
    port_no = atoi(argv[2]);
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        error("Error in opening socket");
    }

    // Connecting to host
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "Error in identifying the host.\n");
        exit(0);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    // Setting the server family
    serv_addr.sin_family = AF_INET;

    memcpy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    // Convert port number to host byte order
    serv_addr.sin_port = htons(port_no);
    if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Error connecting to server.");
    }

    while (1)
    {
        memset(&buffer, 0, sizeof(buffer));
        // bzero(buffer, 256);
        num = read(socket_fd, buffer, 255);
        if (num < 0)
        {
            error("Error in reading from socket.");
        }

        printf("%s\n",buffer);

        if(strcmp(buffer, "Disconnecting from server.") == 0){
            break;
        }

        memset(&buffer, 0, sizeof(buffer));
        fgets(buffer, 255, stdin);
       
        int num = write(socket_fd, buffer, strlen(buffer));
        if (num < 0)
        {
            error("Error in writing to socket");
        }
       
        
    }
    close(socket_fd);
    return 0;
}
