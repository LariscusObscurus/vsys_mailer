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
#include <iostream>
#include <vector>
#include <unistd.h>

void signalHandler(int s)
{
	std::cout << "Socket not connected. Are you banned?" << std::endl;
	std::exit(0);
}

int main(int argc, char **argv){
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sigfillset(&sa.sa_mask);
	sa.sa_handler = signalHandler;
	sigaction(SIGPIPE,&sa,nullptr);

	if(argc < 3) {
		std::cout << "Server and/or Portnumber given!\nusage:" << argv[0] << " hostname port" << std::endl;
		return EXIT_FAILURE;
	}

	char *hostname = argv[1];
	char *port = argv[2];

	Client cli = Client();

	std::cout << "Connecting to Server" << std::endl;


	std::vector<std::string> lines = {
	"---------------------------------",
	"|                               |",
	"|      TWMAILER v0.1.2          |",
	"|      Schwarz/Wuerrer          |",
	"|                               |",
	"|                               |",
	"|           Welcome!            |",
	"|         Please Login:         |",
	"|                               |",
	"---------------------------------",
	};

	for(int i = 0; i < (int)lines.size(); ++i) {
		std::cout << lines[i];
		std::cout << std::endl;
	}

	/*
	LOGIN
	cli.Recieve();
	*/

	std::string buffer;
	std::string message;
	std::string fileName;


	int userOption;
	bool check = false, needData = false;
	if(cli.Connect(hostname, port) == -1) {
		std::cout << "Could not connect to Server" << std::endl;
	}
	for(int i = 0; i <= 2; i++) {
		std::string loginMessage = "LOGIN \n";
		std::cout << "Please Login: " << std::endl << "Username: ";
		std::getline(std::cin,buffer);
		loginMessage  += buffer + "\n";

		char * tmp = getpass("Password: ");
		buffer = std::string(tmp);
		loginMessage += buffer + "\n" + ".\n";
		buffer.clear();

		if(cli.SendMessage(loginMessage) == -1) {
			std::cout << "Could not Login to Server" << std::endl;
		}
		cli.receiveData();

		int rv;
		if((rv = cli.checkOK())) {
			check = true;
			std::cout << "Login OK" << std::endl;
			break;
		} else if(rv == -1) {
			std::cout << "An error ocurred." << std::endl;
			loginMessage.clear();
		}
		std::cout << "Wrong username or password" << std::endl;
	}
	if(!check) {
		std::cout << "Bannend from Server" << std::endl;
		return false;
	}

	while (check)
	{
		std::cout << "Willkommen! Welche Operation wuerden Sie gerne ausfuehren? 1.SEND, 2.LIST, 3.READ, 4.DEL  0.Exit \n";
		std::cin >> userOption;
		switch(userOption) {
		case 1 :
			std::cout << "Bitte geben Sie den Sender ein:(max. 8 chars)";
			message += "SEND\n";
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
			std::cout << "Wollen Sie einen Anhang hinzufügen?: /home/User/...";
			std::getline(std::cin,fileName);
			break;
		case 2 :
			std::cout << "Bitte geben Sie den gewünschten Username ein:(max. 8 chars)";
			message += "LIST\n";
			std::cin.ignore();
			std::getline(std::cin,buffer);
			message += buffer + "\n" + ".\n";
			needData = true;
			break;
		case 3 :
			std::cout << "Bitte geben Sie den gewünschten Username ein:(max. 8 chars)";
			message += "READ\n";
			std::cin.ignore();
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie die Nummer der Nachricht an:(max. 8 chars)";
			std::getline(std::cin,buffer);
			message += buffer + "\n" + ".\n";
			needData = true;
			break;
		case 4 :
			std::cout << "Bitte geben Sie den gewünschten Username ein:(max. 8 chars)";
			message += "DEL\n";
			std::cin.ignore();
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie die Nummer der Nachricht an:(max. 8 chars)";
			std::getline(std::cin,buffer);
			message += buffer + "\n" + ".\n";
			needData = true;
			break;
		case 0 :
			check = false;
			cli.SendMessage("QUIT\n.\n");
			break;
		default:
			std::cout << "Sie müssen eine Zahl zwischen 1 und 4 für die jeweilige Operation eingeben.";
			break;
		}
		if(check ==true){
			if(fileName.length() == 0) {
				cli.SendMessage(message);
			} else {
				cli.SendMessage(message, fileName);
			}
			if(needData) {
				cli.receiveData();
				cli.printMessage();
				needData = false;
			}
			message.clear();
			fileName.clear();
		}
	}

	//cli.Recieve();

		return EXIT_SUCCESS;
}
