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
#include <stdbool.h>
#include <sys/time.h>

#define MAX_BUFFER 255
#define DEFAULT_PORT 12345
#define MAX_CLIENT_QUEUE 5
#define SOCKET_ADDRESS struct sockaddr

// Stuctures
struct Message
{
    char *message;
    int sender_id;
    long int time;
    int *read_by;
};

struct Channel
{
    int channel_id;
    struct Message **messages;
    bool is_subscribed;
    int message_count;
    int message_capacity;
};

struct Client
{
    int client_id;
    int client_code;
    struct Channel **channels;
    int channel_count;
    int channel_capacity;
};

// Global variables
int socket_server;
int client_unique_id = 1;
struct Channel channels[256];

/**
 * Exit function 
 */
void signalCallbackHandler(int signum)
{
    printf("Closing server.\n");
    close(socket_server);

    printf("Server closed.\n");
    // Exit program
    exit(signum);
}

/**
 * Initialises the Client
 * Assigns all new structures with channels
 */ 
struct Client initialiseClient(){
    int len, socket_client;
    struct sockaddr_in cli;
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
        client_unique_id++,
        malloc(sizeof(struct Channel)), // Allocating initial memory to have 1 channel
        0,
        -1};
    // returns the client structure
    return cl;
}

/**
 * Check the Client in the given channel
 */
int checkClientInChannel(struct Client *cl, int channel_id)
{
    
    for (int counter = 0; counter < cl->channel_count; counter++)
    {
        if (cl->channels[counter]->channel_id == channel_id)
        {
            return counter;
        }
    }
    return -1;
}

/**
 * Dynamic array push method for channel
 */
void pushChannel(struct Client *cl, int channel_id)
{
    if (cl->channel_count > cl->channel_capacity)
    {
        // Re allocating memory with channel's size
        int *response = realloc(cl->channels, sizeof(struct Channel) * cl->channel_count);
        cl->channel_capacity = sizeof(struct Channel) * cl->channel_count;
    }

    cl->channels[cl->channel_count] = &channels[channel_id];
    cl->channel_count += 1;
}


/**
 * Subscribe client to channel function
 */
int subClientToChannel(struct Client *cl, char *buffer, char *error_message)
{
    char *value;
    value = strtok(buffer, " ");
    value = strtok(NULL, " ");
    int channel_id = atoi(value);
    
    if (channel_id >= 0 && channel_id < 256)
    {
        int index = checkClientInChannel(cl, channel_id);
        if (index > -1)
        {
            
            if (cl->channels[index]->is_subscribed == false)
            {
                memset(buffer, 0, MAX_BUFFER);
                sprintf(buffer, "Subscribed to channel %d.\n", cl->channels[index]->channel_id);
                cl->channels[index]->is_subscribed = true;
                return 1;
            }
            else
            {
                sprintf(error_message, "Already subscribed to channel %d.\n", channel_id);
                return -1;
            }
        }
        else
        {
            pushChannel(cl, channel_id);
            memset(buffer, 0, MAX_BUFFER);
            sprintf(buffer, "Subscribed to channel %d.\n", cl->channels[cl->channel_count - 1]->channel_id);
            cl->channels[cl->channel_count - 1]->is_subscribed = true;
            return 1;
        }
    }
    else
    {
        sprintf(error_message, "Invalid channel: %d.\n", channel_id);
        return -1;
    }
}

/**
 * Unsubscribe client to channel function
 */
int unsubClientToChannel(struct Client *cl, char *buffer, char *error_message)
{
    char *value;
    value = strtok(buffer, " ");
    value = strtok(NULL, " ");
    int channel_id = atoi(value);
    if (channel_id >= 0 && channel_id < 256)
    {
        int index = checkClientInChannel(cl, channel_id);
        if (index == -1)
        {
            sprintf(error_message, "Not subscribed to channel %d.\n", channel_id);
            return -1;
        }
        else
        {
            if (cl->channels[index]->is_subscribed == false)
            {
                sprintf(error_message, "Not subscribed to channel %d.\n", channel_id);
                return -1;
            }
            else
            {
                cl->channels[index]->is_subscribed = false;
                memset(buffer, 0, MAX_BUFFER);
                sprintf(buffer, "Unsubscribed from channel %d.\n", cl->channels[index]->channel_id);
                return 1;
            }
        }
    }
    else
    {
        sprintf(error_message, "Invalid channel: %d.\n", channel_id);
        return -1;
    }
}


/**
 * Send message to the channel function
 */
int sendMessageToChannel(struct Client *cl, char *buffer, char *error_message)
{
    char *value, *response;
    value = strtok(buffer, " ");
    value = strtok(NULL, " ");
    response = strtok(NULL, " ");
    int channel_id = atoi(value);
    if (channel_id >= 0 && channel_id < 256)
    {
        int index = checkClientInChannel(cl, channel_id);
        if (index == -1)
        {
            sprintf(error_message, "Not subscribed to channel %d.\n", channel_id);
            return -1;
        }
        else
        {
            if (cl->channels[index]->is_subscribed == false)
            {
                sprintf(error_message, "Not subscribed to channel %d.\n", channel_id);
                return -1;
            }
            else
            {
               
                memset(buffer, 0, MAX_BUFFER);
                channels[channel_id].message_count += 5;

                sprintf(buffer, "Message %s %d %d.\n", response, channels[channel_id].message_count, cl->channels[channel_id]->message_count );
                return 1;
                // if (channels[index].message_count > channels[index].message_capacity)
                // {
                //     // Re allocating memory with channel's size
                //     int *response = realloc(channels->messages, sizeof(struct Message) * channels->message_count);
                //     channels->message_capacity = sizeof(struct Message) * channels->message_count;
                // }

                // channels[index].messages[channels->message_count].message = value;
                // channels[index].messages[channels->message_count].time = time(NULL);
                // channels[index].message_count += 1; 

                // cl->channels[index].is_subscribed = false;
                // memset(buffer, 0, MAX_BUFFER);
                // sprintf(buffer, "Unsubscribed from channel %d.\n", cl->channels[index].channel_id);
                // return 1;

            }
        }
    }
    else
    {
        sprintf(error_message, "Invalid channel: %d.\n", channel_id);
        return -1;
    }
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
int checkClientCommand(struct Client *cl, char *buffer, char *error_message)
{
    // Setting error message all zeros
    memset(error_message, 0, MAX_BUFFER);
    if (strncmp("BYE", buffer, 3) == 0) // BYE command
    {
        close(cl->client_id);
        return 3;
    }
    else if (strncmp("SUB", buffer, 3) == 0) // SUB command
    {
        return subClientToChannel(cl, buffer, error_message);
    }
    else if (strncmp("UNSUB", buffer, 5) == 0) // UNSUB command
    {
        return unsubClientToChannel(cl, buffer, error_message);
    }
    else if (strncmp("SEND", buffer, 4) == 0) // SEND command
    {
        //return sendMessageToChannel(cl, buffer, error_message);
    }
}

/**
 * Chat function
 * Will handle the client communication 
*/
void chat(struct Client *cl)
{
    char buff[MAX_BUFFER];
    char error_message[MAX_BUFFER];
    int n;
    int socket_client = cl->client_id;

    // Sending the welcome message to client
    memset(buff, 0, MAX_BUFFER);
    sprintf(buff, "Welcome! Your client ID is %d.\n", cl->client_code);
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
        else if(response == 3)
        {
            *cl = initialiseClient();
            memset(buff, 0, MAX_BUFFER);
            sprintf(buff, "Welcome! Your client ID is %d.\n", cl->client_code);
            write(cl->client_id, buff, sizeof(buff));
            continue;
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
        signal(SIGINT, signalCallbackHandler);
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
    signal(SIGINT, signalCallbackHandler);

     // Initialising channels with no messages
    for (int counter = 0; counter < 256; counter++)
    {
        channels[counter].channel_id = counter;
        channels[counter].messages = malloc(sizeof(struct Message));
        channels[counter].is_subscribed = false;
        channels[counter].message_count = 1;
        channels[counter].message_capacity = 0;
    }

    // Initialise new client
    struct Client cl = initialiseClient();
    
    // Function for chatting between client and server
    chat(&cl);

    // After chatting closing the socket
    close(socket_server);
}