/*
* Process Management and Distributed Computing
* Queensland University of Technology
* September 2019
*
* Server Implementation Header File
* Ruchina Aidan Perera
*
*/

#ifndef SERVER_H
#define SERVER_H

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
#include <stdbool.h>
#include <time.h>

#define MAX_BUFFER 1035
#define DEFAULT_PORT 12345
#define MAX_CLIENT_QUEUE 5
#define SOCKET_ADDRESS struct sockaddr

// Stuctures

struct Client
{
    int client_socket_id;
    int client_code;
    int subscribed_channels[256];
    long int subscribed_time[256];
    int subscribed_read_count[256];
};

typedef struct message
{
    char *content;
    int sender_id;
    long int time;
} Message;

struct Channel
{
    int channel_id;
    Message **messages;
    int message_count;
    int message_capacity;
};

// Global variables
int socket_server;
int client_unique_id = 1;
struct Channel channels[256];
char **loop_buffer;

// Function definitions

/**
 * Exit function 
 */
void signalCallbackHandler(int signum);

/**
 * Initialises the Client
 * Assigns all new structures with channels
 */
struct Client initialiseClient();

/**
 * Check the Client in the given channel
 */
int checkClientInChannel(struct Client *cl, int channel_id);

/**
 * Dynamically allocate memory to the messenges in Channels
 */
void pushMessageToChannel(struct Client *cl, int channel_id, char *message_from_client);

/**
 * Subscribe client to channel function
 */
int subClientToChannel(struct Client *cl, char *buffer, char *error_message);

/**
 * Unsubscribe client to channel function
 */
int unsubClientToChannel(struct Client *cl, char *buffer, char *error_message);

/**
 * Send message to the channel function
 */
int sendMessageToChannel(struct Client *cl, char *buffer, char *error_message);

/**
 * Display the channel list with tab delimeter
 */
void displayChannelList(struct Client *cl, char *buffer);

/**
 * Get the next channel message for the given channel
 */
int getNextChannelMessage(struct Client *cl, int channel_id, char *buffer);

/**
 * Get the next message of a given channel or get the next 
 * message for any channel
 */
int getNextMessage(struct Client *cl, char *buffer, char *error_message);

/**
 * Get the live feed messages of a given channel or get the next 
 * message for any channel
 */
int getLiveFeed(struct Client *cl, char *buffer, char *error_message);

/**
 * Client command check
 * Return:
 *  0 : All good
 *  -1 : Error
 *  1 : Continue the loop
 *  2 : Break the loop
 *  3 : Reinitiate a new client
 *  4 : Print in a loop
 *   
 */
int checkClientCommand(struct Client *cl, char *buffer, char *error_message);

/**
 * Chat function
 * Will handle the client communication 
*/
void chat(struct Client *cl);

#endif