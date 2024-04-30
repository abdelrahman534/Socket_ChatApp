#include "socketutil.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 1024

typedef struct {
    int sockfd;
    char username[50];
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
int numClients = 0;

void *handleClient(void *arg) {
    ClientInfo *client = (ClientInfo *)arg;
    char buffer[MAX_MSG_SIZE];

    while (1) {
        ssize_t bytesReceived = recv(client->sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            printf("%s: %s\n", client->username, buffer);
            
            
            // Forward message to all other clients
            for (int i = 0; i < numClients; i++) {
                if (clients[i].sockfd != client->sockfd) {
                    ssize_t sentMessage = send(clients[i].sockfd, buffer, bytesReceived, 0);
                    if (sentMessage == -1) {
                        printf("Error sending data to client\n");
                        break;
                    }
                }
            }
        } else if (bytesReceived == 0) {
            printf("Client disconnected: %s\n", client->username);
            break;
        } else {
            printf("Error receiving data from client: %s\n", client->username);
            break;
        }
    }

    // Remove client from the list
    for (int i = 0; i < numClients; i++) {
        if (clients[i].sockfd == client->sockfd) {
            for (int j = i; j < numClients - 1; j++) {
                clients[j] = clients[j + 1];
            }
            if(numClients == 0){
                printf("Empty Chat...Closing Connection.\n");
                exit(EXIT_FAILURE);
            }
            numClients--;
            break;
        }
    }

    close(client->sockfd);
    free(client);
    pthread_exit(NULL);
}

int main() {
    // Create tcp socket
    int serversocketfd = CreateTcpIpv4Socket();
    if (serversocketfd < 0) {
        printf("Error creating socket\n");
        return 1;
    }

    // Create address
    struct sockaddr_in *serverAddress = CreateIPv4Address("INADDR_ANY", 2000);
    if (serverAddress == NULL) {
        printf("Error creating server address\n");
        close(serversocketfd);
        return 1;
    }

    // Bind
    int result = bind(serversocketfd, (struct sockaddr *)serverAddress, sizeof(struct sockaddr_in));
    if (result != 0) {
        printf("Error binding socket\n");
        free(serverAddress);
        close(serversocketfd);
        return 1;
    } else {
        printf("Socket was bound successfully!!\n");
    }

    // Listen
    int listenResult = listen(serversocketfd, 10);
    if (listenResult != 0) {
        printf("Error listening on socket\n");
        free(serverAddress);
        close(serversocketfd);
        return 1;
    }

    printf("Server is listening for incoming connections...\n");

    struct sockaddr_in clientaddress;
    socklen_t clientaddressSize = sizeof(struct sockaddr_in);

    while (1) {
        int clientsocketfd = accept(serversocketfd, (struct sockaddr *)&clientaddress, &clientaddressSize);
        if (clientsocketfd < 0) {
            printf("Error accepting client connection!!\n");
            continue;
        }

        // Receive username length from client
        size_t lengthReceived;
        ssize_t usernameLengthReceived = recv(clientsocketfd, &lengthReceived, sizeof(lengthReceived), 0);
        if (usernameLengthReceived <= 0) {
            printf("Error receiving username length from client!!\n");
            close(clientsocketfd);
            continue;
        }

        // Validate the received length
        if (lengthReceived <= 0 || lengthReceived > MAX_MSG_SIZE) {
            printf("Invalid username length received from client!!\n");
            close(clientsocketfd);
            continue;
        }

        char *username = (char *)malloc(lengthReceived + 1); // +1 is for the null terminator
        if (username == NULL) {
            printf("Error allocating memory for username!!\n");
            close(clientsocketfd);
            continue;
        }

        // Receive username from client
        ssize_t usernameReceived = recv(clientsocketfd, username, lengthReceived, 0);
        if (usernameReceived <= 0) {
            printf("Error receiving username from client!!\n");
            close(clientsocketfd);
            free(username);
            continue;
        }

        username[usernameReceived] = '\0';
        printf("Client connected: %s\n", username);

        // Add client to the list
        if (numClients < MAX_CLIENTS) {
            clients[numClients].sockfd = clientsocketfd;
            strncpy(clients[numClients].username, username, sizeof(clients[numClients].username) - 1);
            clients[numClients].username[sizeof(clients[numClients].username) - 1] = '\0';
            numClients++;

            // Create a new thread to handle the client
            ClientInfo *clientInfo = (ClientInfo *)malloc(sizeof(ClientInfo));
            clientInfo->sockfd = clientsocketfd;
            strncpy(clientInfo->username, username, sizeof(clientInfo->username) - 1);
            clientInfo->username[sizeof(clientInfo->username) - 1] = '\0';
            pthread_t tid;
            pthread_create(&tid, NULL, handleClient, (void *)clientInfo);
            pthread_detach(tid);
        } else {
            printf("Maximum clients reached. Rejecting connection.\n");
            close(clientsocketfd);
        }
    }

    free(serverAddress);
    close(serversocketfd);

    return 0;
}
