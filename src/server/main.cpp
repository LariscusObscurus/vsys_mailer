#include <cstdlib>
#include <iostream>
#include "Server.h"
#include "ServerException.h"
#include <cstring>
#include <signal.h>
#include <sys/wait.h>

/**
 * run = false fährt den Server ordnungsgemäß herunter
 * count zählt SIGINT
 * count >= 2 Notabschaltung
 */
bool run = true;
int count =  0;

/**
 * SIGCHLD
 */
void signalHandler(int s)
{
	while(waitpid(-1,NULL, WNOHANG) > 0);
}
/**
 * SIGINT
 */
void StopServer(int s)
{
	run = false;
	if(count++ >= 2) std::exit(0);
}

int main(int argc, char *argv[])
{
	struct sigaction sa, sa_stop;
	memset(&sa_stop,0,sizeof(sa_stop));

	/**
	 * signalHandler für SIGINT
	 */
	sigfillset(&sa.sa_mask);
	sa_stop.sa_handler = StopServer;
	sigaction(SIGINT,&sa_stop,NULL);

	if(argc < 3) {
		std::cout << "No Server Path and/or Portnumber given!" << std::endl;
		return EXIT_FAILURE;
	}

	Server serv = Server(argv[1]);
	std::cout << "Starting Server" << std::endl;
	try
	{
		serv.Connect(NULL, argv[2]);
	} catch(const ServerException& ex)
	{
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	/**
	 * signalHandler für SIGCHLD 
	 */
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1) {
		std::cout << "sigaction" << std::endl;
		exit(1);
	}

	std::cout << "Listening..." << std::endl;
	serv.Start(&run);
	waitpid(-1,0,0);
	return EXIT_SUCCESS;
}
