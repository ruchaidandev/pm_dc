/*
* Process Management and Distributed Computing
* Queensland University of Technology
* September 2019
*
* Server Implementation
* Ruchina Aidan Perera
*
*/

#include "server.h"

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
struct Client initialiseClient()
{
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
        {0}, // Allocating zeros
        {0},
    };
    // returns the client structure
    return cl;
}

/**
 * Check the Client in the given channel
 */
int checkClientInChannel(struct Client *cl, int channel_id)
{
    if (cl->subscribed_channels[channel_id] == 1)
    {
        return 1;
    }

    return -1;
}

/**
 * Dynamically allocate memory to the messenges in Channels
 */
void pushMessageToChannel(struct Client *cl, int channel_id, char *message_from_client)
{

    if (channels[channel_id].message_count > channels[channel_id].message_capacity)
    {
        // Re allocating memory with messages's size
        int *response = realloc(channels[channel_id].messages, sizeof(Message) * channels[channel_id].message_count);
        channels[channel_id].message_capacity += 1;
    }

    int message_index = channels[channel_id].message_count;
    Message msg = {
        message_from_client, // message
        cl->client_code,     // sender
        time(NULL),          // time
        0                    // read by
    };
    channels[channel_id].messages[message_index] = &msg;
    channels[channel_id].message_count += 1;
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
        if (index == 1)
        {
            sprintf(error_message, "Already subscribed to channel %d.\n", channel_id);
            return -1;
        }
        else
        {
            memset(buffer, 0, MAX_BUFFER);
            sprintf(buffer, "Subscribed to channel %d.\n", channel_id);
            cl->subscribed_channels[channel_id] = 1;
            cl->subscribed_time[channel_id] = time(NULL);
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
        if (index < 1)
        {
            sprintf(error_message, "Not subscribed to channel %d.\n", channel_id);
            return -1;
        }
        else
        {
            cl->subscribed_channels[channel_id] = 0;
            cl->subscribed_time[channel_id] = 0;
            memset(buffer, 0, MAX_BUFFER);
            sprintf(buffer, "Unsubscribed from channel %d.\n", channel_id);
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
 * Send message to the channel function
 */
int sendMessageToChannel(struct Client *cl, char *buffer, char *error_message)
{
    char *value, *message_from_client;
    value = strtok(buffer, " ");
    value = strtok(NULL, " ");
    // Channel number
    int channel_id = atoi(value);
    // Message
    value = strtok(NULL, " ");
    while (value != NULL)
    {
        strcat(message_from_client, value);
        strcat(message_from_client, " ");
        value = strtok(NULL, " ");
    }

    if (channel_id >= 0 && channel_id < 256)
    {
        int index = checkClientInChannel(cl, channel_id);
        if (index < 1)
        {
            sprintf(error_message, "Not subscribed to channel %d.\n", channel_id);
            return -1;
        }
        else
        {
            // Check message in range from 0 to 1024
            if (strlen(message_from_client) > 0 && strlen(message_from_client) <= 1024)
            {
                pushMessageToChannel(cl, channel_id, message_from_client);
                memset(buffer, 0, MAX_BUFFER);
                return 1;
            }
            else
            {
                sprintf(error_message, "Error: message can not be empty or greater than 1024 characters.\n");
                return -1;
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
 * Display the channel list with tab delimeter
 */
int displayChannelList(struct Client *cl, char *buffer)
{
    int loop_counter = 0; // Counts the loop buffer
   
    for (int counter = 0; counter < 256; counter++)
    {
        if (cl->subscribed_channels[counter] == 1)
        {
            memset(buffer, 0, MAX_BUFFER);
            sprintf(buffer, "LINE||%d\t%d\t%d\t%d\n", counter, channels[counter].message_count, 0, 0);
            send(cl->client_id, buffer, strlen(buffer), 0);
            loop_counter += 1;
        }
    }

    // write(cl->client_id, buff, strlen(buff));
  
    return loop_counter;
}

/**printf("%s\n",response);
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
        return sendMessageToChannel(cl, buffer, error_message);
    }
    else if (strncmp("CHANNELS", buffer, 8) == 0) // CHANNELS command
    {
        memset(buffer, 0, MAX_BUFFER);
        sprintf(buffer, "LOOP||");
        send(cl->client_id, buffer, 4, 0);
        
        memset(buffer, 0, MAX_BUFFER);
        int total = displayChannelList(cl, buffer);
        return 4;
    }
    else
    {
        sprintf(error_message, "Error: Invalid command.\n");
        return -1;
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
        else if (response == 3)
        {
            *cl = initialiseClient();
            memset(buff, 0, MAX_BUFFER);
            sprintf(buff, "Welcome! Your client ID is %d.\n", cl->client_code);
            write(cl->client_id, buff, sizeof(buff));
            continue;
        }
        else if (response == 4)
        {
            memset(buff, 0, MAX_BUFFER);
            sprintf(buff, "END||");
            write(cl->client_id, buff, 4);
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
        channels[counter].messages = malloc(sizeof(Message));
        channels[counter].message_count = 0;
        channels[counter].message_capacity = -1;
    }

    // Initialise new client
    struct Client cl = initialiseClient();

    // Function for chatting between client and server
    chat(&cl);

    // After chatting closing the socket
    close(socket_server);
}