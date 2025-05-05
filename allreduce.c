#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int program = 2; //to check which program you are running from 
int scatterCheck = 0; //if 1 means reduce is done, if 2 means scatter is done 
int array[4]; //array you are adding together
pthread_mutex_t lock;
pthread_cond_t condition; 

struct threadArgs {
    int port; //specific to server and client 
};

void* serverHandle(void* args) {
    //get the input 
    struct threadArgs* serverArgs = (struct threadArgs*) args;
    int server_fd, clientSocket; 
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    //change if array size different 
    int recv[2];

    //create server and socket stuff 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Failed to make server socket");
        exit(EXIT_FAILURE);
    }

    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(serverArgs->port);
    address.sin_family = AF_INET;
    
    if (bind(server_fd, (struct sockaddr*) &address, sizeof(address)) < 0) {
        perror("Failed to bind to server");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Failed to listen");
        exit(EXIT_FAILURE);
    }

    if ((clientSocket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*)&addrlen)) < 0) {
        perror("Failed to accepet connection");
        exit(EXIT_FAILURE);
    }

    read(clientSocket, recv, sizeof(recv));

    //Server of programs 1 and 2 recieve and sum the first or last two elements here
    pthread_mutex_lock(&lock);
    if (program == 1) {
        array[2] += recv[0];
        array[3] += recv[1];
    }
    else {
        array[0] += recv[0];
        array[1] += recv[1];
    }
    
    scatterCheck++;
    //To check after reduce phase
    printf("Final array after reduce phase: ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
    //signal that scatter finished 
    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&lock);

    //check if scatter is done to end off the thread 
    pthread_mutex_lock(&lock);
    while (scatterCheck != 2) {
        pthread_cond_wait(&condition, &lock);
    }

    //if program 1 it will send last two digits, and if program 2 it will send first two digits 
    if (program == 1) {
        write(clientSocket, &array[2], sizeof(int) * 2);
    }
    else {
        write(clientSocket, &array[0], sizeof(int) * 2);
    }
    pthread_mutex_unlock(&lock);

    close(clientSocket);
    close(server_fd);
    //return to main
    return NULL;
}

void *clientHandle(void* args) {
    //get the input 
    struct threadArgs* clientArgs = (struct threadArgs*) args;
    int client_fd = 0;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    //for program 2 to send last two elements  
    int send[2] = {array[2], array[3]};

    //create client server stuff
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create client socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(clientArgs->port);

    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0) {
        perror("Error creating address for client");
        exit(EXIT_FAILURE);
    }

    //decide what to send for reduce depending on the program number
    while (connect(client_fd, (struct sockaddr*) &address, addrlen) < 0) {
        pthread_mutex_lock(&lock);
        if (program == 2) {
            program = 1;
            send[0] = array[0];
            send[1] = array[1];
        }
        pthread_mutex_unlock(&lock);
    }

    write(client_fd, send, sizeof(send));
    //check if reduce is done
    pthread_mutex_lock(&lock);
    while (scatterCheck != 1) {
        pthread_cond_wait(&condition, &lock);
    }
    scatterCheck++;
    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&lock);
    
    //here is the scatter phase update the respective indices based on the program
    if (program == 1) {
        read(client_fd, &array[0], sizeof(int) * 2);
    }
    else {
        read(client_fd, &array[2], sizeof(int) * 2);
    }
    close(client_fd);
    //return to main 
    return NULL;
}

int main(int argc, char const *argv[]) {

    if (argc != 7) {
        printf("You need to have at least 7 arguments, clientPORT, serverPORT, [1], [2], [3], [4] \n");
        exit(EXIT_FAILURE);
    }

    //Initialize mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex initialization failed\n");
        exit(EXIT_FAILURE);
    }

    //Initialize condition variable
    if (pthread_cond_init(&condition, NULL) != 0) {
        printf("Condition variable initialization failed\n");
        pthread_mutex_destroy(&lock);
        exit(EXIT_FAILURE);
    }

    int serverPORT = atoi(argv[1]);
    int clientPORT = atoi(argv[2]);

    pthread_t serverTid, clientTid;
    struct threadArgs serverArgs, clientArgs;

    serverArgs.port = serverPORT;
    clientArgs.port = clientPORT; 

    printf("Starting array: ");
    for (int i = 0; i < 4; i++) {
        array[i] = atoi(argv[i+3]);
        printf("%d ", array[i]);
    }
    printf("\n");

    //create and wait for threads to finish reduce-scatter
    pthread_create(&serverTid, NULL, serverHandle, &serverArgs);
    pthread_create(&clientTid, NULL, clientHandle, &clientArgs);

    pthread_join(serverTid, NULL);
    pthread_join(clientTid, NULL);

    printf("The final array after scatter phase: ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
    
    exit(EXIT_SUCCESS);
}
