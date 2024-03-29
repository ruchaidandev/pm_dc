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

    // Delete shared memory
    shmctl(shm_id, IPC_RMID, NULL);
    shmctl(client_shm_id, IPC_RMID, NULL);

    sem_destroy(&mutex);
    sem_destroy(&writers_lock);

    // Exit program
    exit(signum);
}

void *reader(void *arg){

}

void *writer(void *arg){
    int client_id = (int) arg;

    sem_wait(&writers_lock);
    
}

/**
 * Accepting and waiting for new clients
 */
int listenForClients(){
    int socket_client;
    socklen_t len;
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

    return socket_client;
} 

/**
 * Initialises the Client
 * Assigns all new structures with channels
 */
struct Client initialiseClient(int socket_client, int client_id)
{
    // Setting new client
    struct Client cl = {
        socket_client,
        0, // Pthread id
        client_id,
        {0}, // Allocating zeros
        {0},
        {0}};
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
 * Dynamically allocate memory to the connected clients
 */
void updateConnectedClients(struct Client *cl)
{
    // Process synchronisation
    sem_wait(&mutex);
    // Critical section
    if (connectedClients.client_count > connectedClients.client_capacity)
    {
        // Re allocating memory with messages's size
        int *response = realloc(connectedClients.clients, sizeof(struct Client) * connectedClients.client_count);
        if(response > 0){
           connectedClients.client_capacity += 1;
        }
    }
    connectedClients.clients[connectedClients.client_count] = cl;
    connectedClients.client_count += 1;
    sem_post(&mutex);
}

/**
 * Dynamically allocate memory to the messenges in Channels
 */
void pushMessageToChannel(channel *channels, struct Client *cl, int channel_id, char *message_from_client)
{
    pthread_mutex_lock(&mutex_lock);
    if (channels[channel_id].message_count > channels[channel_id].message_capacity)
    {
        // Re allocating memory with messages's size
        int *response = realloc(channels[channel_id].messages, sizeof(Message) * channels[channel_id].message_count);
        if(response > 0){
            channels[channel_id].message_capacity += 1;
        }
    }

    int message_index = channels[channel_id].message_count;

    // Creating a new message
    Message *msg = malloc(sizeof(Message));
    msg->sender_id = cl->client_code;
    msg->content = malloc(sizeof(char) * 1024);
    strncpy(msg->content, message_from_client, 1024);
    msg->time = time(NULL);
    channels[channel_id].messages[message_index] = msg;
    channels[channel_id].message_count += 1;
    cl->subscribed_read_count[channel_id] += 1;
    pthread_mutex_unlock(&mutex_lock);

}

/**
 * Subscribe client to channel function
 */
int subClientToChannel(channel *channels, struct Client *cl, char *buffer, char *error_message)
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
int unsubClientToChannel(channel *channels, struct Client *cl, char *buffer, char *error_message)
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
int sendMessageToChannel(channel *channels, struct Client *cl, char *buffer, char *error_message)
{
    char *value;
    value = strtok(buffer, " ");
    value = strtok(NULL, " ");
    // Channel number
    int channel_id = atoi(value);
    // Message
    value = strtok(NULL, "");

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
            // 1026 is to avoid the \n \0 trailing in the message.
            if (value != NULL && strlen(value) > 0 && strlen(value) < 1026)
            {
                pushMessageToChannel(channels, cl, channel_id, value);
                memset(buffer, 0, MAX_BUFFER);
                return 1;
            }
            else
            {
                sprintf(error_message, "Message can not be empty or greater than 1024 characters.\n");
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
void displayChannelList(channel *channels, struct Client *cl, char *buffer)
{
    for (int counter = 0; counter < 256; counter++)
    {
        if (cl->subscribed_channels[counter] == 1)
        {
            int message_count_after_subscribed = 0;
            for (int itr = 0; itr < channels[counter].message_count; itr++)
            {
                // Checks the time of subscribed
                // Counts the total messages client did not read
                if (channels[counter].messages[itr]->time > cl->subscribed_time[counter])
                {
                    message_count_after_subscribed += 1;
                }
            }
            memset(buffer, 0, MAX_BUFFER);
            sprintf(buffer, "|LL|%d\t%d\t%d\t%d\n", counter, channels[counter].message_count,
                    cl->subscribed_read_count[counter],
                    message_count_after_subscribed - cl->subscribed_read_count[counter]);
            write(cl->client_socket_id, buffer, strlen(buffer));
        }
    }
}



/**
 * Get the next channel message for the given channel
 */
int getNextChannelMessage(channel *channels, struct Client *cl, int channel_id, char *buffer, bool print_channel)
{
    // For all messages in the channel
    for (int itr = 0; itr < channels[channel_id].message_count; itr++)
    {
        // Checks the time of subscribed
        if (channels[channel_id].messages[itr]->time > cl->subscribed_time[channel_id])
        {
            for (int read_itr = cl->subscribed_read_count[channel_id];
                 read_itr < channels[channel_id].message_count; read_itr++)
            {
                cl->subscribed_read_count[channel_id] += 1;
                memset(buffer, 0, MAX_BUFFER);
                // Checking whether to print the channel id
                if(print_channel == true){
                    sprintf(buffer, "|LL|%d:%s", channel_id, channels[channel_id].messages[read_itr]->content);
                }else{
                    sprintf(buffer, "|LL|%s", channels[channel_id].messages[read_itr]->content);
                }
                write(cl->client_socket_id, buffer, strlen(buffer));
                return 4;
            }
        }
    }
    return 4;
}

/**
 * Get the next message of a given channel or get the next 
 * message for any channel
 */
int getNextMessage(channel *channels, struct Client *cl, char *buffer, char *error_message)
{
    char *value;
    value = strtok(buffer, " ");

    // When a channel is id provided
    value = strtok(NULL, " ");

    // When no channel is provided
    if (value == NULL)
    {
        bool not_subscribed_to_any = true;
        // Iterating over all channels
        for(int itr =0; itr < 256; itr++){
            if(cl->subscribed_channels[itr] == 1){
                not_subscribed_to_any = false;
                // For all messages in the channel
                getNextChannelMessage(channels, cl, itr, buffer, true);
                memset(buffer, 0, MAX_BUFFER);
                return 4;
            }
        }
        
        if(not_subscribed_to_any == true){
            sprintf(error_message, "Not subscribed to any channels.\n");
            return -1;
        }
    }
    else
    {
        
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
                // For all messages in the channel
                getNextChannelMessage(channels, cl, channel_id, buffer, false);
                memset(buffer, 0, MAX_BUFFER);
                return 4;
            }
        }
        else
        {
            sprintf(error_message, "Invalid channel: %d.\n", channel_id);
            return -1;
        }
    }

    return 0;
}

/**
 * Get the live feed messages of a given channel or get the next 
 * message for any channel
 */
int getLiveFeed(channel *channels, struct Client *cl, char *buffer, char *error_message)
{
    char *value;
    value = strtok(buffer, " ");
    // When a channel is id provided
    value = strtok(NULL, " ");
    
    // When no channel is provided
    if (value == NULL)
    {
        bool not_subscribed_to_any = true;
        for(int itr =0; itr < 256; itr++){
            if(cl->subscribed_channels[itr] == 1){
                not_subscribed_to_any = false;
            }
        }
        if(not_subscribed_to_any == true){
            sprintf(error_message, "Not subscribed to any channels.\n");
            return -1;
        }

        write(cl->client_socket_id, "OK", 2);
        memset(buffer, 0, MAX_BUFFER);

        // Iterating over all channels
        while (true)
        {
            
            for(int itr =0; itr < 256; itr++){
                if(cl->subscribed_channels[itr] == 1){
                    /**
                     * Reading break point from client
                     */
                    memset(buffer, 0, MAX_BUFFER);
                    read(cl->client_socket_id, buffer, sizeof(buffer));
                    if (strncmp("OK", buffer, 2) == 0)
                    {
                        memset(buffer, 0, MAX_BUFFER);
                        break;
                        // Exit loop
                    }
                    getNextChannelMessage(channels, cl, itr, buffer, false);
                }
            }
           
        }
        return 5;
       
    }
    else
    {
        
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
                write(cl->client_socket_id, "OK", 2);
                memset(buffer, 0, MAX_BUFFER);
                // For all messages in the channel
                while (true)
                {
                    
                    /**
                     * Reading break point from client
                     */
                    memset(buffer, 0, MAX_BUFFER);
                    read(cl->client_socket_id, buffer, sizeof(buffer));
                    if (strncmp("OK", buffer, 2) == 0)
                    {
                        memset(buffer, 0, MAX_BUFFER);
                        break;
                        // Exit loop
                    }
                    getNextChannelMessage(channels, cl, channel_id, buffer, false);
                }
                return 5;
            }
        }
        else
        {
            sprintf(error_message, "Invalid channel: %d.\n", channel_id);
            return -1;
        }
    }
}

/**printf("%s\n",response);
 * Client command check
 * Return:
 *  0 : All good
 *  -1 : Error
 *  1 : Continue the loop
 *  2 : Break the loop
 *  3 : Reinitiate a new client
 *  4 : Print in a loop with sending a blank loop line
 *  5 : Print in a loop 
 */
int checkClientCommand(channel *channels, struct Client *cl, char *buffer, char *error_message)
{
    // Setting error message all zeros
    memset(error_message, 0, MAX_BUFFER);
    if (strncmp("BYE", buffer, 3) == 0) // BYE command
    {
        close(cl->client_socket_id);
        return 2;
    }
    else if (strncmp("SUB", buffer, 3) == 0) // SUB command
    {
        return subClientToChannel(channels, cl, buffer, error_message);
    }
    else if (strncmp("UNSUB", buffer, 5) == 0) // UNSUB command
    {
        return unsubClientToChannel(channels, cl, buffer, error_message);
    }
    else if (strncmp("SEND", buffer, 4) == 0) // SEND command
    {
        return sendMessageToChannel(channels, cl, buffer, error_message);
    }
    else if (strncmp("CHANNELS", buffer, 8) == 0) // CHANNELS command
    {
        displayChannelList(channels, cl, buffer);
        memset(buffer, 0, MAX_BUFFER);
        return 4;
    }
    else if (strncmp("NEXT", buffer, 4) == 0) // NEXT command
    {
        return getNextMessage(channels, cl, buffer, error_message);
    }
    else if (strncmp("LIVEFEED", buffer, 8) == 0) // LIVEFEED command
    {
        return getLiveFeed(channels, cl, buffer, error_message);
    }

    return 0; // Return 0 by default
}

/**
 * Chat function
 * Will handle the client communication 
*/
void chat(int socket_client, channel *channels)
{
    char buff[MAX_BUFFER];
    char error_message[MAX_BUFFER];

    // Attaching shared memory for client ID
    client_shm_id = shmget(SHM_KEY_CLIENT_ID, sizeof(int), 0666 | IPC_CREAT);
    if (client_shm_id  < 0) {
         exit(-1);
    }
    
    int *client_unique_id = shmat(client_shm_id, (void *)0, 0);

    //  Process synchronisation 
    sem_wait(&mutex);

    // Critical section
    int value = *client_unique_id + 1;
    *client_unique_id = value;
    // Dettaching the shared memory
    shmdt(client_unique_id); 

    sem_post(&mutex);

    // Creating new client for the process
    struct Client cl = initialiseClient(socket_client, value);

    // Update connected clients
    updateConnectedClients(&cl);

    // Saving the thread ID in the client struct   
    //cl.thread_id = pthread_self();
    
    // Sending the welcome message to client
    memset(buff, 0, MAX_BUFFER);
    sprintf(buff, "Welcome! Your client ID is %d.\n", cl.client_code);
    write(socket_client, buff, sizeof(buff));
    
    // Looping till the server exits
    for (;;)
    {
        // Setting the buffer all zeros
        memset(buff, 0, MAX_BUFFER);

        // Reading the message from the client
        read(socket_client, buff, sizeof(buff));
        // Client command / input check
        int response = checkClientCommand(channels, &cl, buff, error_message);
        if (response == 1)
        {
            // Write to client            
            // Sending loop break char to eliminate waiting for input
            // in client side
            //write(socket_client, "|LL|", 5);
            write(socket_client, buff, sizeof(buff));
            continue;
        }
        else if (response == 2)
        {
            // Close thread
            close(cl.client_socket_id);
            // Detattach memory
            shmdt(shm_id);
            exit(1);

            //return NULL;
        }
        else if (response == 4)
        {
            // Sending loop break char to eliminate waiting for input
            // in client side
            write(socket_client, "|LL|", 5);
            continue;
        }
        else if (response == 5)
        {
            continue;
        }
        else if (response == -1)
        {
            // Write to client
            write(socket_client, error_message, sizeof(error_message));
            continue;
        }
       
        // Write to client
        write(socket_client, buff, sizeof(buff));

        // Detect SIGINT
        signal(SIGINT, signalCallbackHandler);
    }

}

/**
 * Connecting client to the server
 * Will loop till it finds new client and create new thread
 */ 
void connectClient(){
    while(true){
        // Listening for new clients
        int socket_client = listenForClients();
        // If new client
        if(socket_client > 0){
             // Creating new thread
            pthread_t thread_id;
            if(fork() == 0){
                // Creating shared memory for the channels
                // Passing the socket client value 
                shm_id = shmget(SHM_KEY, sizeof(channel) * 256, IPC_CREAT | 0666);
                channel *channels = (channel *) shmat(shm_id, (void *) 0, 0);  // Attaching
                chat(socket_client, channels);
                shmdt(shm_id); // Detaching

            }
            
            //pthread_create(&thread_id, NULL, &chat, &socket_client);
        }   
    }
}

/**
 * Main method of the server
 */
int main(int argc, char *argv[])
{
    // Server address
    struct sockaddr_in servaddr;
    // Shared memory
    channel *channels;

    // Semaphore initialisation
    sem_init(&mutex,0,1);
    sem_init(&writers_lock,0,1);
    // Initialising connected clients
    // Creating with initial size for 2 clients
    // Capacity is 1 by default
    connectedClients.clients = malloc(sizeof(struct Client) * 2);
    connectedClients.client_capacity = 1;
    connectedClients.client_count = 0;

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

    // Shared memory for the client id
    client_shm_id = shmget(SHM_KEY_CLIENT_ID, sizeof(int), 0666 | IPC_CREAT);
    if (client_shm_id  < 0) {
         exit(-1);
    }
    
    int *client_unique_id = (int *) shmat(client_shm_id, (void *) 0, 0);
    
    // Setting default value to the shared client id
    int default_client_id = 0;
    client_unique_id = &default_client_id;
    // Dettaching the shared memory
    shmdt(client_unique_id); 



    // Shared memory for the channels
    shm_id = shmget(SHM_KEY, sizeof(channel) * 256, IPC_CREAT | 0666);
    if (shm_id  < 0) {
         exit(-1);
    }

    channels = (channel *) shmat(shm_id, NULL, 0);
    

    // Initialising channels with no messages
    // Creating with initial size for 10 message per channel
    // Capacity is 9 by default
    for (int counter = 0; counter < 256; counter++)
    {
        channels[counter].channel_id = counter;
        channels[counter].messages = malloc(sizeof(Message) * 10);
        channels[counter].message_count = 0;
        channels[counter].message_capacity = 9;
    }
    shmdt(shm_id); 
    
    // Connect clients to server
    connectClient();

    // After chatting closing the socket
    close(socket_server);

    // Delete shared memory
    shmctl(shm_id, IPC_RMID, NULL);
    shmctl(client_shm_id, IPC_RMID, NULL);
    sem_destroy(&mutex);
    sem_destroy(&writers_lock);
}
