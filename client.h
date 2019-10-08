#ifndef CLIENT_H   
#define CLIENT_H

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


// Function definitions

/**
 * Exit function 
 */
void signalCallbackHandler(int signum);

/**
 * Chat function
 * Will handle the client communication 
*/
void chat();



#endif