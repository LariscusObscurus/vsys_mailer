#include <cstdlib>
#include <iostream>
#include "Server.h"

int main(int argc, char *argv[])
{
	Server serv; 

	std::cout << "Starting Server" << std::endl;
	try
	{
		serv.Connect(NULL, "3490");
	} catch(const char *ex)
	{
		std::cerr << ex << std::endl;
	}
	std::cout << "Listening..." << std::endl;
	serv.Start();
	return 0;
}
