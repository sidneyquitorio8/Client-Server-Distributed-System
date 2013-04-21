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
    int canDo; //0 if you cant do, 1 if the server itself can do it
    //currently the system has a 10 server max. You can change this easily
    char *otherServers[10];
    
};

//global scope variables
//currently the system has a 10 server max. You can change this easily
char *serverList[10];
struct function functionList[10] = {{{'\0'},0, 0, {}}};
int numberofOtherServers;

void processServerHello(int clientSock) {
    int functionFound = 0;
    int holder = 0;
    char serverHelloName[50];
    char functionName[50];
    int functionParam;
    int eof =0;
    int i =0;
    pthread_mutex_t lock;
    char currentLine[50];
    char *currentLinePointer = currentLine;
    char *firstCurrentLineMemory = currentLinePointer;

        //put a mutex just incase 2 servers start up at the same time and try to input their functions to the functionList, and their names to the serverList
        if(pthread_mutex_init(&lock, NULL) != 0) {
            perror("mutex init failed\n");
            exit(0);
        }
        
        while( (recv(clientSock, currentLinePointer, 1, 0) != 0) && (*currentLinePointer != '\n')) {
            currentLinePointer++;
        }
        *currentLinePointer = '\0';
        
        strcpy(serverHelloName, currentLine);
        
        //critical section. protects serverList, numberOfServers, functionList
        pthread_mutex_lock(&lock);
        
        if ( serverList[numberofOtherServers] == NULL ) {
            serverList[numberofOtherServers] = malloc(strlen(currentLine) + 1);
        }
        strcpy(serverList[numberofOtherServers], currentLine);
        numberofOtherServers++;
        //add functions to functionList
        while(eof != 1) {
            i = 0;
            functionFound = 0;
            memset(currentLine, '\0', sizeof(currentLine));
            currentLinePointer = firstCurrentLineMemory;
            while( (recv(clientSock, currentLinePointer, 1, 0) != 0) && ((*currentLinePointer != '\n') && (*currentLinePointer != '\r') )) {
                currentLinePointer++;
            }
            //check if its the end of statement character i made up
            if(*currentLinePointer == '\r') {
                eof =1;
                break;
            }
            *currentLinePointer = '\0';
            
            
            //once you have the function and param in current line, then seperate them and store it.
            currentLinePointer = firstCurrentLineMemory;
            currentLinePointer = strtok(currentLine, " ");
            strcpy(functionName, currentLine);
            currentLinePointer = strtok(NULL, " ");
            functionParam = atoi(currentLinePointer);
            //search and see if the function exist.
            while(strcmp(functionList[i].name, "") != 0 && i >= -1) {
                
                //if the function exist in the functionList already, then just add server to the functions list of capable servers
                if(strcmp(functionList[i].name, functionName) == 0 && functionList[i].parameterNumer == functionParam) {
                    functionFound = 1;
                    holder = i;
                    
                    //function found so go through the functions list of servers and add it to the list
                    i = 0;
                    while(i >= 0) {
                        
                        //allocate memory for the array of strings located at the given function because C sucks
                        if ( functionList[holder].otherServers[i] == NULL ) {
                            // add one for the terminator
                            functionList[holder].otherServers[i] = malloc(strlen(serverHelloName) + 1);
                            // make sure the allocation worked
                            if ( functionList[holder].otherServers[i] == NULL ) {
                                // something went wrong so break;
                                break;
                            }
                            //since it was null, it must have been empty to begin with so set it to null
                            strcpy(functionList[holder].otherServers[i], serverHelloName);
                            i = -10;
                            break;
                        }
                        //if the string in the array was not null, then you should look for when it is empty just in case
                        if(strcmp(functionList[holder].otherServers[i], "") == 0) {
                            strcpy(functionList[holder].otherServers[i], serverHelloName);
                            i = -10; //
                        }
                        //server is already in the functions list of servers, so break out.
                        if(strcmp(functionList[holder].otherServers[i], serverHelloName) == 0) {
                            i = -10;
                            break;
                        }
                        
                        if(i == 9) { //ran through entire list of all possible servers and couldnt find an empty slot
                            printf("server list full, should allow more room for other servers");
                            fflush(stdout);
                            i = -10;
                        }
                        i++;
                        
                    }
                    
                }
                if(i == 9) { //ran through entire list of all possible functions and did not see an empty slot or there is no match
                    printf("function list full so could not add, and there was no match for any functions");
                    fflush(stdout);
                    i = -10;
                }
                i++;
            }
            
            //if the function was not found in the list, we have to add it in
            if(functionFound == 0) {
                i = 0;
                while(strcmp(functionList[i].name, "") != 0) {
                    i++;
                }
                strcpy(functionList[i].name, functionName);
                functionList[i].parameterNumer = functionParam;
                
                //allocate memory
                if ( functionList[i].otherServers[0] == NULL ) {
                    functionList[i].otherServers[0] = malloc(strlen(serverHelloName) + 1);
                }
                strcpy(functionList[i].otherServers[0], serverHelloName);
                
            }
            
            
        }
        pthread_mutex_unlock(&lock);
        pthread_mutex_destroy(&lock);
        
    
}


/**
 Handles each clients request whether it is a server request or a client request
 **/
void request_handler(int clientSock) {
    int functionFound = 0;
    int holder = 0;
    char serverHelloName[50];
    char functionName[50];
    int functionParam;
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
        
        processServerHello(clientSock);
        /**
        //put a mutex just incase 2 servers start up at the same time and try to input their functions to the functionList, and their names to the serverList
        if(pthread_mutex_init(&lock, NULL) != 0) {
            perror("mutex init failed\n");
            exit(0);
        }
        
        while( (recv(clientSock, currentLinePointer, 1, 0) != 0) && (*currentLinePointer != '\n')) {
            currentLinePointer++;
        }
        *currentLinePointer = '\0';
        
        strcpy(serverHelloName, currentLine);
        
        //critical section. protects serverList, numberOfServers, functionList
        pthread_mutex_lock(&lock);
        
        if ( serverList[numberofOtherServers] == NULL ) {
            serverList[numberofOtherServers] = malloc(strlen(currentLine) + 1);
        }
        strcpy(serverList[numberofOtherServers], currentLine);
        numberofOtherServers++;
        //add functions to functionList
        while(eof != 1) {
            i = 0;
            functionFound = 0;
            memset(currentLine, '\0', sizeof(currentLine));
            currentLinePointer = firstCurrentLineMemory;
            while( (recv(clientSock, currentLinePointer, 1, 0) != 0) && ((*currentLinePointer != '\n') && (*currentLinePointer != '\r') )) {
                currentLinePointer++;
            }
            //check if its the end of statement character i made up
            if(*currentLinePointer == '\r') {
                eof =1;
                break;
            }
            *currentLinePointer = '\0';
            
            
            //once you have the function and param in current line, then seperate them and store it.
            currentLinePointer = firstCurrentLineMemory;
            currentLinePointer = strtok(currentLine, " ");
            strcpy(functionName, currentLine);
            currentLinePointer = strtok(NULL, " ");
            functionParam = atoi(currentLinePointer);
            //search and see if the function exist. 
            while(strcmp(functionList[i].name, "") != 0 && i >= -1) {
                
                //if the function exist in the functionList already, then just add server to the functions list of capable servers
                if(strcmp(functionList[i].name, functionName) == 0 && functionList[i].parameterNumer == functionParam) {
                    functionFound = 1;
                    holder = i;
                    
                    //function found so go through the functions list of servers and add it to the list
                    i = 0;
                    while(i >= 0) {
                        
                        //allocate memory for the array of strings located at the given function because C sucks
                        if ( functionList[holder].otherServers[i] == NULL ) {
                            // add one for the terminator
                            functionList[holder].otherServers[i] = malloc(strlen(serverHelloName) + 1);
                            // make sure the allocation worked
                            if ( functionList[holder].otherServers[i] == NULL ) {
                                // something went wrong so break;
                                break;
                            }
                            //since it was null, it must have been empty to begin with so set it to null
                            strcpy(functionList[holder].otherServers[i], serverHelloName);
                            i = -10;
                            break;
                        }
                        //if the string in the array was not null, then you should look for when it is empty just in case
                        if(strcmp(functionList[holder].otherServers[i], "") == 0) {
                            strcpy(functionList[holder].otherServers[i], serverHelloName);
                            i = -10; //
                        }
                        //server is already in the functions list of servers, so break out.
                        if(strcmp(functionList[holder].otherServers[i], serverHelloName) == 0) {
                            i = -10;
                            break;
                        }
                        
                        if(i == 9) { //ran through entire list of all possible servers and couldnt find an empty slot
                            printf("server list full, should allow more room for other servers");
                            fflush(stdout);
                            i = -10;
                        }
                        i++;
                        
                    }
                    
                }
                if(i == 9) { //ran through entire list of all possible functions and did not see an empty slot or there is no match
                    printf("function list full so could not add, and there was no match for any functions");
                    fflush(stdout);
                    i = -10;
                }
                i++;
            }
            
            //if the function was not found in the list, we have to add it in
            if(functionFound == 0) {
                i = 0;
                while(strcmp(functionList[i].name, "") != 0) {
                    i++;
                }
                strcpy(functionList[i].name, functionName);
                functionList[i].parameterNumer = functionParam;
                
                //allocate memory
                if ( functionList[i].otherServers[0] == NULL ) {
                    functionList[i].otherServers[0] = malloc(strlen(serverHelloName) + 1);
                }
                strcpy(functionList[i].otherServers[0], serverHelloName);
                
            }
            
            
        }
        pthread_mutex_unlock(&lock);
        pthread_mutex_destroy(&lock);
         **/
        
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

