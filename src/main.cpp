#include <cstdlib>
#include <iostream>
#include "Server.h"
#include <signal.h>
#include <sys/wait.h>

void signalHandler(int s)
{
	while(waitpid(-1,NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[])
{
	Server serv = Server(argv[1]); 
	struct sigaction sa;
	if(argc < 3) {
		std::cout << "No Server Path and/or Portnumber given!" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Starting Server" << std::endl;
	try
	{
		serv.Connect(NULL, argv[2]);
	} catch(const char *ex)
	{
		std::cerr << ex << std::endl;
		return EXIT_FAILURE;
	}

	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1) {
		std::cerr << "sigaction" << std::endl;
		exit(1);
	}

	std::cout << "Listening..." << std::endl;
	serv.Start();
	return EXIT_SUCCESS;
}
