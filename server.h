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
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <semaphore.h>

#define MAX_BUFFER 1035
#define DEFAULT_PORT 12345
#define MAX_CLIENT_QUEUE 5
#define SOCKET_ADDRESS struct sockaddr
#define SHM_KEY 0x1234
#define SHM_KEY_CLIENT_ID 0x2345

// Stuctures

struct Client
{
    int client_socket_id;
    pthread_t thread_id;
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

typedef struct Channel
{
    int channel_id;
    Message **messages;
    int message_count;
    int message_capacity;
} channel;

struct ConnectedClients
{
    struct Client **clients;
    int client_count;
    int client_capacity;
} connectedClients;

// Global variables
int socket_server;
int shm_id;
int client_shm_id;
// Semaphore locks
sem_t mutex, writers_lock;
int readers_count = 0;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

// Function definitions

/**
 * Exit function 
 */
void signalCallbackHandler(int signum);

/**
 * Accepting and waiting for new clients
 */
int listenForClients();

/**
 * Initialises the Client
 * Assigns all new structures with channels
 */
struct Client initialiseClient(int socket_client, int client_id);

/**
 * Check the Client in the given channel
 */
int checkClientInChannel(struct Client *cl, int channel_id);

/**
 * Dynamically allocate memory to the connected clients
 */
void updateConnectedClients(struct Client *cl);

/**
 * Dynamically allocate memory to the messenges in Channels
 */
void pushMessageToChannel(channel *channels, struct Client *cl, int channel_id, char *message_from_client);

/**
 * Subscribe client to channel function
 */
int subClientToChannel(channel *channels, struct Client *cl, char *buffer, char *error_message);

/**
 * Unsubscribe client to channel function
 */
int unsubClientToChannel(channel *channels, struct Client *cl, char *buffer, char *error_message);

/**
 * Send message to the channel function
 */
int sendMessageToChannel(channel *channels, struct Client *cl, char *buffer, char *error_message);

/**
 * Display the channel list with tab delimeter
 */
void displayChannelList(channel *channels, struct Client *cl, char *buffer);

/**
 * Get the next channel message for the given channel
 */
int getNextChannelMessage(channel *channels, struct Client *cl, int channel_id, char *buffer, bool print_channel);

/**
 * Get the next message of a given channel or get the next 
 * message for any channel
 */
int getNextMessage(channel *channels, struct Client *cl, char *buffer, char *error_message);

/**
 * Get the live feed messages of a given channel or get the next 
 * message for any channel
 */
int getLiveFeed(channel *channels, struct Client *cl, char *buffer, char *error_message);

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
int checkClientCommand(channel *channels, struct Client *cl, char *buffer, char *error_message);

/**
 * Chat function
 * Will handle the client communication 
*/
void chat(int socket_client, channel *channels);

/**
 * Connecting client to the server
 * Will loop till it finds new client and create new thread
 */
void connectClient();

#endif