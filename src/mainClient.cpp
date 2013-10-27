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
#include <stdio.h>
#include <string.h>


int main(int argc, char **argv){

	if(argc < 3) {
		std::cout << "Server and/or Portnumber given!\nusage:" << argv[0] << " hostname port" << std::endl;
		return EXIT_FAILURE;
	}

	char *hostname = argv[1];
	char *port = argv[2];

	Client cli = Client();

	std::cout << "Connecting to Server" << std::endl;


	std::string buffer;
	std::string message;
	//bzero((char *) &serv_addr, sizeof(serv_addr));
	int userOption;
	bool check;
	check = true;
	while (check)
	{
		std::cout << "Willkommen! Welche Operation wuerden Sie gerne ausfuehren? 1.SEND, 2.LIST, 3.READ, 4.DEL  0.Exit \n";
		std::cin >> userOption;
		switch(userOption) {
		case 1 :
			std::cout << "Bitte geben Sie den Sender ein:(max. 8 chars)";
			message += "SEND \n";
			std::cin.ignore();
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie den Empfaenger ein:(max. 8 chars)";
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie den Betreff ein:(max. 80 chars)";
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie die Nachricht ein:(max. 920 chars)";
			std::getline(std::cin,buffer);
			message += buffer + "\n" + ".\n";
			break;
		case 2 :
			std::cout << "Bitte geben Sie den gewünschten Username ein:(max. 8 chars)";
			message += "LIST \n";
			std::cin.ignore();
			std::getline(std::cin,buffer);
			message += buffer + "\n" + ".\n";
			break;
		case 3 :
			std::cout << "Bitte geben Sie den gewünschten Username ein:(max. 8 chars)";
			message += "READ \n";
			std::cin.ignore();
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie die Nummer der Nachricht an:(max. 8 chars)";
			std::getline(std::cin,buffer);
			message += buffer + "\n" + ".\n";
			break;
		case 4 :
			std::cout << "Bitte geben Sie den gewünschten Username ein:(max. 8 chars)";
			message += "DEL \n";
			std::cin.ignore();
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie die Nummer der Nachricht an:(max. 8 chars)";
			std::getline(std::cin,buffer);
			message += buffer + "\n" + ".\n";
			break;
		case 0 :
			check = false;
			break;
		default:
			std::cout << "Sie müssen eine Zahl zwischen 1 und 4 für die jeweilige Operation eingeben.";
			break;
		}
		cli.Connect(hostname, port);
		cli.SendMessage(message.c_str(),message.length());
		message.clear();
	}

	//cli.ReadMessage();

		return EXIT_SUCCESS;
}
