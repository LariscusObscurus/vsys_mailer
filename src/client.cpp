/**
 * @file client.cpp
 * @author Florian Wuerrer <if13b077@technikum-wien.at>
 * @date 11.10.2013
 *
 * @brief client for client-server application
 * 
 **/

/** include **/
#include "client.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fstream>

/** macros **/
/** MAXLINE LENGTH**/
#define LINEMAX 1024
/** Error message for close**/
#define CLOSEERRORMSG "close error\n"
/** Error message for fclose**/
#define FCLOSEERRORMSG "fclose error\n"



int sockfd = -1;

Client::Client() {
}

Client::~Client(){
	close(sockfd);
}

int Client::Connect (char* const server_hostname, const char* server_service){
        struct addrinfo* ai,* ai_sel = NULL;
        struct addrinfo hints;
        int err;

        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_addrlen = 0;
        hints.ai_addr = NULL;
        hints.ai_canonname = NULL;
        hints.ai_next = NULL;

        //resolve ipaddr
        if((err = getaddrinfo(server_hostname, server_service, &hints, &ai)) != 0){
                (void) fprintf(stderr, "ERROR:  %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }

        if(ai == NULL){
                (void) fprintf(stderr, "Could not resolve host %s.\n", server_hostname);
                freeaddrinfo(ai);
                exit(EXIT_FAILURE);
        }

        ai_sel = ai;

        //creat socket
        if((sockfd = socket(ai_sel->ai_family, ai_sel->ai_socktype, ai->ai_protocol)) < 0){
                (void) fprintf(stderr, "Socket creation failed\n");
                close(sockfd);
                freeaddrinfo(ai);
                exit(EXIT_FAILURE);
        }

        //connect to server
        if(connect(sockfd, ai_sel->ai_addr, ai_sel->ai_addrlen) < 0){
                (void) close(sockfd);
                freeaddrinfo(ai);
                (void) fprintf(stderr, "Connection failed.\n");
                close(sockfd);
                exit(EXIT_FAILURE);
        }
        
	freeaddrinfo(ai);
        //freeaddrinfo(ai_sel);

	return sockfd;
}

int Client::SendMessage(char* const message,int size){
	int bytes_sent = write(sockfd, message, size);
                if(bytes_sent < 0){
                        printf("Write Error.");
                        (void) close(sockfd);
                        exit(EXIT_FAILURE);
                }
        
	return 0;
}
