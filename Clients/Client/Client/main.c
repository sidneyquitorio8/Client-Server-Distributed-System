//
//  main.c
//  Client
//
//  Created by Sidney Quitorio on 4/20/13.
//  Copyright (c) 2013 Γ080 Sidney Quitorio. All rights reserved.
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

int main(int argc, const char * argv[])
{
    int i = 0;
    char receive[1024] = "";
    char wholeRequest[1024] = "";
    char command[1024] = "";
    int over = 0;
    char *localIP;
    struct hostent *he;
    char buf[1024] = "";
    
    int client_sock_desc;
    int len;
    struct sockaddr_in server_addr;


    char *pointer;
    char servername[50] = "";
    int serverport;
    
    strcpy(buf, argv[1]);
    
    pointer = strtok(buf, ":");
    strcpy(servername, buf);
    pointer = strtok(NULL, ":");
    serverport = atoi(pointer);

    printf("You are going to be connected to %s:%d\n", servername, serverport);
    printf("If you would like to specify another server to connect to, please enter \"CONNECT\"\n");
    printf("If you would like to quit please enter \"QUIT\"\n");
    printf("Otherwise, please enter the function along with parameters seperated by a space\n");
    //start processing user commands
    while(over == 0) {
        printf("Enter command:\n");
        fgets(buf, sizeof(buf), stdin);
        strcpy(wholeRequest, buf);
        pointer = strtok(buf, " ");
        strcpy(command, buf);
        if(strcmp(command, "CONNECT\n") == 0) {
            printf("Please specify other server using format: hostname:portNumber\n");
            fgets(buf, sizeof(buf), stdin);
            pointer = strtok(buf, ":");
            strcpy(servername, buf);
            pointer = strtok(NULL, ":");
            serverport = atoi(pointer);
            printf("You are going to be connected to %s:%d\n", servername, serverport);
            printf("If you would like to specify another server to connect to, please enter \"CONNECT\"\n");
            printf("If you would like to quit please enter \"QUIT\"\n");
            printf("Otherwise, please enter the function along with parameters seperated by a space\n\n");
        }
        else if(strcmp(buf, "QUIT\n") == 0) {
            over = 1;
            printf("Goodbye\n");
            break;
        }
        else if(strcmp(buf, "") != 0) { //must be a function so connect to the server
            //begin setting up connection
            if((he = gethostbyname(servername)) == NULL ) {
                
                perror(strcat("Cannot find server named:", servername));
                exit(0);
            }
            
            he = gethostbyname("localhost");
            localIP = inet_ntoa(*(struct in_addr *)*he->h_addr_list);
            client_sock_desc = socket(AF_INET, SOCK_STREAM, 0);
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = inet_addr(localIP);
            server_addr.sin_port = htons(serverport);
            len = sizeof(server_addr);
            if(connect(client_sock_desc, (struct sockaddr *)&server_addr,len) == -1) {
                perror("Client failed to connect");
                exit(0);
            }
            //WORST PROGRAMMING LANGUAGE EVER. HACKY
            for(i = 0; i < 1024; i++) {
                buf[i]='\0';
            }
            strcpy(buf, "CLIENT/REQUEST\n");
            send(client_sock_desc, buf, strlen(buf), 0);
            //send actual function request
            
            //put a space before \n char to make it easier for the server
            for(i = 0; i < sizeof(wholeRequest); i++) {
                if(wholeRequest[i] == '\n') {
                    wholeRequest[i] = ' ';
                    wholeRequest[i+1] = '\n';
                    wholeRequest[i+2] = '\0';
                    /**
                    for(i = i+2; i < 1024; i++) { //erase entire buffer. this causes an error if you dont. c sucks!
                       wholeRequest[i] = '\0'; 
                    }
                     **/
                    break;
                     
                }
            }
            
            strcpy(buf, wholeRequest);
            send(client_sock_desc, buf, strlen(buf), 0);
            
            while((i = read(client_sock_desc, receive, sizeof(receive)) > 0)) {
                printf(receive);
                fflush(stdout);
                strcpy(receive, "");
                receive[i] = '\0'; //even adding a null terminating character wont fix this bug.
                //WORST PROGRAMMING LANGUAGE EVER. WHY DO I HAVE TO DO SUCH A HACKY FIX!?!?!?
                for(i = 0; i < 1024; i++) {
                    receive[i]='\0';
                }
            }
            
            
            
            close(client_sock_desc);
            
            
        }
        else {
            printf("Invalid please try again");
            break;
        }
        
    }
    
    return 0;
}

