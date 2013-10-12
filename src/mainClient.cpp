/**
 * @file mainClient.cpp
 * @author Florian Wuerrer <if13b077@technikum-wien.at>
 * @date 11.10.2013
 *
 * @brief client-main for client-server application
 * 
 **/

/** include **/
#include <cstdlib>
#include <iostream>
#include "client.h"
#include <signal.h>
#include <sys/wait.h>
#include <string>

/** macros **/
/** MAXLINE LENGTH**/
#define LINEMAX 1024
/** Error message for close**/
#define CLOSEERRORMSG "close error\n"
/** Error message for fclose**/
#define FCLOSEERRORMSG "fclose error\n"


void signalHandler(int s){
	while(waitpid(-1,NULL, WNOHANG) > 0);
}

int main(int argc, char **argv){	 
	struct sigaction sa;

	if(argc < 3) {
		std::cout << "Server and/or Portnumber given!\nusage:" << argv[0] << " hostname port" << std::endl;
		return EXIT_FAILURE;
	}

	char *hostname = argv[1];
	char *port = argv[2];
	
	Client cli = Client();

	std::cout << "Connectiing to Server" << std::endl;
	try{
		cli.Connect(hostname, port);

		char message[1024];
  		std::cout << "Please, enter your name(max. 1024 chars): ";
  		std::cin.getline (message,1024);

		cli.SendMessage(message,sizeof(message));

	} catch(const char *ex){
		std::cerr << ex << std::endl;
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
