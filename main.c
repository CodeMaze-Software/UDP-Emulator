//
//  main.c
//  Emulator
//
//  Created by Dariusz Adamczyk on 16/08/2020.
//  Copyright © 2020 Dariusz Adamczyk. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "emulator.h"

#define MAX_BUFFER_LEN          32
#define MAX_PORT_LEN            7

#define EXAMPLE_MAX_RELAYS_NUM  2

int  sockfd;
socklen_t len;
struct sockaddr_in servaddr, cliaddr;

// Application callback
void sendDataToClientCallback(unsigned int * data)
{
    sendto(sockfd, data, MAX_BUFFER_LEN, 0, (const struct sockaddr *) &cliaddr, len);
}

int main() {
    
    int  emulator = ON;
    unsigned int buffer[MAX_BUFFER_LEN];
    char port[MAX_PORT_LEN];
    
    // Create socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
      
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    
    // Type UDP port
    printf("Type emulator port...\r\n");
    scanf("%6s", port);
    
    // Check if the port is valid
    if(portIsValid(port) == OK)
        printf("Typed port is: %s\r\n", port);
    else
        exit(EXIT_FAILURE);
    
    // Fill server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
      
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    len = sizeof(cliaddr); 
    
    printf("Waiting for the client....\r\n");
    
    // Register application callback
    registerEmulatorCallback(sendDataToClientCallback);
    
    // Initialize emulator and set number of relays
    emulatorInit(NUM_OF_USED_RELAYS);
    
    while(emulator)
    {
        // Receive and analyze the received datagram
        recvfrom(sockfd, buffer, MAX_BUFFER_LEN, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        
        switch(processIncommingPacket(buffer))
        {
            case APP_CLOSE:
                printf("Stop the emulator\r\n");
                emulator = OFF;
                break;
                
            case UNKNOWN_CODE:
                printf("Unknown code\r\n");
                break;
                
            case RANGE_ERROR:
                printf("Range error\r\n");
                break;
                
            case SYNTAX_ERROR:
                printf("Syntax error\r\n");
                break;
            
            case SEQ_ERROR:
                printf("Sequence error\r\n");
                break;
                
        }
    }
    
    // Close socket and exit application
    close(sockfd);
    
    return 0;
}
