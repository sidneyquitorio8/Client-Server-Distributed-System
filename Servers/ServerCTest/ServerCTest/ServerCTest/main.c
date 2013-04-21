//
//  main.c
//  functionPointer
//
//  Created by Sidney Quitorio on 4/19/13.
//  Copyright (c) 2013 Sidney Quitorio. All rights reserved.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 20
struct function {
    char name[20];
    int parameterNumer;
    int canDo;
    char *otherServers[10];
    
};

//global scope variables
char *serverList[10];
struct function functionList[10] = {{"",0, 0, {}}};
int numberofOtherServers;


/**
 Handles each clients request whether it is a server request or a client request
 **/
void request_handler(int clientSock) {
    char holder;
    int eof =0;
    int i =0;
    pthread_mutex_t lock;
    char currentLine[50];
    char *currentLinePointer = currentLine;
    char *firstCurrentLineMemory = currentLinePointer;
    char command[50];
    char *request = command;
    while( (recv(clientSock, request, 1, 0) != 0) && (*request != '\n')) {
        request++;
    }
    *request = '\0';
    
    printf("got here\n");
    fflush(stdout);
    
    //check if it is SERVER/HELLO command
    if(strcmp(command, "SERVER/HELLO") == 0) {
        //put a mutex just incase 2 servers start up at the same time and try to input their functions to the functionList, and their names to the serverList
        if(pthread_mutex_init(&lock, NULL) != 0) {
            perror("mutex init failed\n");
            exit(0);
        }
        
        while( (recv(clientSock, currentLinePointer, 1, 0) != 0) && (*currentLinePointer != '\n')) {
            currentLinePointer++;
        }
        *currentLinePointer = '\0';
        
        //critical section. protects serverList, numberOfServers, functionList
        pthread_mutex_lock(&lock);
        serverList[numberofOtherServers] = currentLine;
        numberofOtherServers++;
        //add functions to functionList
        while(eof != 1) {
            memset(currentLine, '\0', sizeof(currentLine));
            currentLinePointer = firstCurrentLineMemory;
            while( (recv(clientSock, currentLinePointer, 1, 0) != 0) && ((*currentLinePointer != '\n') && (*currentLinePointer != '\r') )) {
                currentLinePointer++;
            }
            if(*currentLinePointer == '\r') {
                eof =1;
            }
            *currentLinePointer = '\0';
            
            //FIX THIS TO NOT ONLY DO FUNCTIONS WITH 1 LETTER WORDS!!!! TRY THIS OUT IMMEDIATELY
            //FIX THIS TO NOT ONLY DO FUNCTIONS WITH 1 LETTER WORDS!!!! TRY THIS OUT IMMEDIATELY
            //FIX THIS TO NOT ONLY DO FUNCTIONS WITH 1 LETTER WORDS!!!! TRY THIS OUT IMMEDIATELY
            //FIX THIS TO NOT ONLY DO FUNCTIONS WITH 1 LETTER WORDS!!!! TRY THIS OUT IMMEDIATELY
            //FIX THIS TO NOT ONLY DO FUNCTIONS WITH 1 LETTER WORDS!!!! TRY THIS OUT IMMEDIATELY
            //FIX THIS TO NOT ONLY DO FUNCTIONS WITH 1 LETTER WORDS!!!! TRY THIS OUT IMMEDIATELY
            //FIX THIS TO NOT ONLY DO FUNCTIONS WITH 1 LETTER WORDS!!!! TRY THIS OUT IMMEDIATELY
            //FIX THIS TO NOT ONLY DO FUNCTIONS WITH 1 LETTER WORDS!!!! TRY THIS OUT IMMEDIATELY
            //if the function given exist, then add the server to the functions list of servers, if it doesn't then add the function
            while(strcmp(functionList[i].name, "") != 0) {
                holder = currentLine[0];
                if(functionList[i].name) {
                    printf("yay");
                    fflush(stdout);
                }
                
                i++;
            }
            
            
        }
        pthread_mutex_unlock(&lock);
        pthread_mutex_destroy(&lock);
        
    }
    
}

int main(int argc, const char * argv[])
{
    char holder[50];
    char hostName[50];
    char whoiam[50];
    char *localIP;
    struct hostent *he;
    char *serverpointer;
    char server[50];
    int serverPort=0;
    char buf[1024];
    
    //server acting as client
    int server_send_sock_desc;
    int len;
    struct sockaddr_in destination_addr;
    
    int server_sock_desc;
    struct sockaddr_in server_name;
    
    int client_sock_desc;
    struct sockaddr_in client_name;
    socklen_t addr_size;
    pthread_t handler_thread;
    
    
    int i =0;
    int portNumber = 0;
    char *configFile = argv[1];
    FILE *fp;
    
    if(argv[1] == NULL) {
        perror("Must set config file");
        exit(0);
    }
    //Server configuration
    fp = fopen(configFile, "r");
    if(fp == NULL) {
        perror("File not found");
        exit(1);
    }
    fscanf(fp, "%s", hostName);
    strcpy(whoiam, hostName);
    serverpointer = strtok(hostName, ":");
    serverpointer = strtok(NULL, ":");
    portNumber = atoi(serverpointer);
    while(fscanf(fp, "%s %d", functionList[i].name, &functionList[i].parameterNumer) == 2) {
        functionList[i].canDo = 1;
        i++;
    }
    //Stores the names of the other servers given as parameters
    i = 2;
    while(argv[i] != NULL) {
        serverList[i-2] = argv[i];
        i++;
    }
    numberofOtherServers = i-2;
    
    
    
    /**
     Setup connection to send HELLO message to other servers
     */
    //Create send socket for the HELLO message
    for(i = 0; i < numberofOtherServers; i++) {
        strcpy(holder, serverList[i]);
        serverpointer = strtok(holder, ":");
        strcpy(server, serverpointer);
        serverpointer = strtok(NULL, ":");
        serverPort = atoi(serverpointer);
        
        if((he = gethostbyname(server)) == NULL ) {
            
            perror(strcat("Cannot find server named:", server));
            exit(0);
        }
        
        he = gethostbyname("localhost");
        localIP = inet_ntoa(*(struct in_addr *)*he->h_addr_list);
        server_send_sock_desc = socket(AF_INET, SOCK_STREAM, 0);
        destination_addr.sin_family = AF_INET;
        destination_addr.sin_addr.s_addr = inet_addr(localIP);
        destination_addr.sin_port = htons(serverPort);
        len = sizeof(destination_addr);
        if(connect(server_send_sock_desc, (struct sockaddr *)&destination_addr,len) == -1) {
            perror("Failed to connect to send HELLO message");
            exit(0);
        }
        //send server hello messae
        strcpy(buf, "SERVER/HELLO\n");
        send(server_send_sock_desc, buf, strlen(buf), 0);
        strcat(whoiam, "\n");
        strcpy(buf, whoiam);
        send(server_send_sock_desc, buf, strlen(buf), 0);
        for(i = 0; i < strcmp(functionList[i].name, "") != 0; i++) {
            strcpy(buf, "");
            strcat(buf, functionList[i].name);
            sprintf(buf, "%s %d\n", buf, functionList[i].parameterNumer);
            send(server_send_sock_desc, buf, strlen(buf), 0);
            
        }
        strcpy(buf, "\r");
        send(server_send_sock_desc, buf, strlen(buf), 0);
        //close socket. important!
        close(server_send_sock_desc);
    }
    
    
    
    
    
    /**
     Connection setup ad wait for connections
     */
    server_sock_desc = socket(PF_INET, SOCK_STREAM, 0);
    if(server_sock_desc != -1) {
        memset(&server_name, 0, sizeof(server_name));
        server_name.sin_family = AF_INET;
        server_name.sin_port = htons(portNumber);
        server_name.sin_addr.s_addr = htonl(INADDR_ANY);
        int bind_result = bind(server_sock_desc, (struct sockaddr *) &server_name, sizeof(server_name));
        if(bind_result == 0) {
            if(listen(server_sock_desc, BACKLOG) < 0) {
                perror("listen failed");
            }
            
            addr_size = sizeof(client_name);
            
            //Server Loop will continue to run listening for clients connecting to the server
            while(1) {
                
                //new client attempting to connect to the server
                client_sock_desc = accept(server_sock_desc, (struct sockaddr *) &client_name, &addr_size);
                if(client_sock_desc == -1) {
                    if(errno == EINTR) {
                        continue;
                    }
                    else {
                        perror("accept failed");
                        exit(1);
                    }
                }
                
                //connection starts here
                //create a thread for the new clients request to be handled
                if(pthread_create(&handler_thread, NULL, request_handler, client_sock_desc) != 0) {
                    perror("pthread_create failed");
                }
            }
        }
        else {
            perror("bind failed");
        }
    }
    else {
        perror("socket failed");
    }
    
    
    
    return 0;
    
}

