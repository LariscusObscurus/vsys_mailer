#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argv, char** argc){


//Variablen initialisierung - beginn ***
    int host_port= 5846; //Iein freier Port
    char* host_name="127.0.0.1"; //Localhost halt

    struct sockaddr_in my_addr;

    char buffer[1024];
    int bytecount;
    int buffer_len=0;

    int hsock;
    int * p_int;
    int err;
// end ***


//Socketaufbau
    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        printf("Error initializing socket %d\n",errno);
        goto FINISH;
    }
    
    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;
        
    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        printf("Error setting options %d\n",errno);
        free(p_int);
        goto FINISH;
    }
    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);
    
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = inet_addr(host_name);

//Connect zum Server
    if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        if((err = errno) != EINPROGRESS){
            fprintf(stderr, "Error connecting socket %d\n", errno);
            goto FINISH;
        }
    }





//Die Message an den Server, also die eigentliche Nachricht in der Mail
//Werd ich noch erweitern mit Betreff, bzw. dennoch alles in einen Buffer hauen und mit einer bestimmten Zeichenfolge     //abgrenzen. (z.B. Ich bin ein Betreff|||Ich bin die Nachricht)

    buffer_len = 1024;
    memset(buffer, '\0', buffer_len);

    printf("Gib bitte den Text deiner Mail an. (press enter)\n");
    fgets(buffer, 1024, stdin);
    buffer[strlen(buffer)-1]='\0';
    
    if( (bytecount=send(hsock, buffer, strlen(buffer),0))== -1){
        fprintf(stderr, "Error sending data %d\n", errno);
        goto FINISH;
    }
	
    printf("Sent bytes %d\n", bytecount);

    if((bytecount = recv(hsock, buffer, buffer_len, 0))== -1){
        fprintf(stderr, "Error receiving data %d\n", errno);
        goto FINISH;
    }
	
    printf("Recieved bytes %d\nReceived string \"%s\"\n", bytecount, buffer);

    close(hsock);
    
FINISH:
;
}


//Soll ich meinen Server (hab ich nur zum testen geschrieben, also nix großes) auch raufladen`?
