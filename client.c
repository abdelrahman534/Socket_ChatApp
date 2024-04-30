#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "socketutil.h"

#define MAX_MSG_SIZE 1024

// Structure to pass arguments to the thread functions
struct ThreadArgs {
    int sockfd;
    char username[50];
};

// Thread function to send messages to the server
void *sendMessage(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    char message[MAX_MSG_SIZE];

    while (1) {
        // Read message from the user
        printf("%s : ", args->username);
        fgets(message, sizeof(message), stdin);
        printf("\n");
        // Send the message to the server
        ssize_t amountSent = send(args->sockfd, message, strlen(message), 0);
        if (amountSent == -1) {
            printf("Error sending message to server\n");
            break;
        }
    }

    return NULL;
}

// Thread function to receive messages from the server
void *receiveMessage(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    char buffer[MAX_MSG_SIZE];

    while (1) {
        // Receive message from server
        ssize_t amountReceived = recv(args->sockfd, buffer, sizeof(buffer) - 1, 0);
        if (amountReceived > 0) {
            buffer[amountReceived] = '\0';
            printf("\nOTHER_USER : %s\n", buffer);
        } else if (amountReceived == 0) {
            printf("Server disconnected.\n");
            break;
        } else {
            printf("Error receiving message from server.\n");
            break;
        }
    }

    return NULL;
}

int main() {
    // Create TCP socket
    int sockfd = CreateTcpIpv4Socket();
    if (sockfd < 0) {
        printf("Error creating socket\n");
        return 1;
    }

    // Create server address
    struct sockaddr_in *address = CreateIPv4Address("127.0.0.1", 2000);
    if (address == NULL) {
        printf("Error creating server address\n");
        close(sockfd);
        return 1;
    }

    // Connect to the server
    int result = connect(sockfd, (struct sockaddr *)address, sizeof(*address));
    if (result != 0) {
        printf("Connection failed!!\n");
        free(address);
        close(sockfd);
        return 1;
    }

    printf("Connection Established!!\n");

    // Prompt for username
    char username[50];
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0'; // Remove the newline character

    // Send username length
    size_t usernameLen = strlen(username);
    ssize_t usernameLenSent = send(sockfd, &usernameLen, sizeof(usernameLen), 0);
    if (usernameLenSent == -1) {
        printf("Error sending username length to server\n");
        free(address);
        close(sockfd);
        return 1;
    }

    // Send username to server
    ssize_t usernameSent = send(sockfd, username, strlen(username) + 1, 0); // Include null terminator
    if (usernameSent == -1) {
        printf("Error sending username to server\n");
        free(address);
        close(sockfd);
        return 1;
    }

    printf("Welcome, %s!!\n", username);

    // Create thread arguments
    struct ThreadArgs args;
    args.sockfd = sockfd;
    strcpy(args.username, username);

    // Create threads for sending and receiving messages
    pthread_t sendThread, receiveThread;
    pthread_create(&sendThread, NULL, sendMessage, (void *)&args);
    pthread_create(&receiveThread, NULL, receiveMessage, (void *)&args);

    // Wait for threads to finish
    pthread_join(sendThread, NULL);
    pthread_join(receiveThread, NULL);

    free(address);
    close(sockfd);
    return 0;
}
