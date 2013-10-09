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
	Server serv; 
	struct sigaction sa;

	std::cout << "Starting Server" << std::endl;
	try
	{
		serv.Connect(NULL, "3490");
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
