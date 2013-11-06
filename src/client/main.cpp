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


int main(int argc, char **argv){

	if(argc < 3) {
		std::cout << "Server and/or Portnumber given!\nusage:" << argv[0] << " hostname port" << std::endl;
		return EXIT_FAILURE;
	}

	char *hostname = argv[1];
	char *port = argv[2];

	Client cli = Client();

	std::cout << "Connecting to Server" << std::endl;

	int login;
        int userOption;
        int count;
        count = 0;
        
 	bool useroptcheck;
	useroptcheck = true;
	bool logincheck;
	logincheck = true;
        
        std::string loginerror;
   	std::string buffer;
	std::string message;
	std::string fileName;
	std::string loginMessage = "LOGIN \n";
	std::string registerMessage = "REGISTER \n";


        
	std::vector<std::string> loginout = {
        "\t\t~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~",
        "\t\t|----         TWMailer           ----|",
        "\t\t~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~",
        "\t\t",
        "\t\t",
        "\t\t   .:::::::::::::::::::::::::::::::::.",
        "\t\t   .::| 1.Login                   |::.",
        "\t\t   .::|                           |::.",
        "\t\t   .::| 2.Register                |::.",
        "\t\t   .::|                           |::.",
        "\t\t   .::| 0.Exit                    |::.",
        "\t\t   .:::::::::::::::::::::::::::::::::.\n",
	};

	for(int i = 0; i < loginout.size(); ++i) {
		std::cout << loginout[i];
		std::cout << std::endl;
	}
   
        while (logincheck){
	std::cin >> login;
	switch(login)
            {
                    case 1:
                    std::cout << "Username: \n";
		    std::cin.ignore();
	            std::getline(std::cin,buffer);
                    loginMessage  += buffer + "\n";
                    std::cout << "Password: \n";
                    std::getline(std::cin,buffer);
                    loginMessage  += buffer + "\n" + ".\n";
                    buffer.clear();
                    break;

                    case 2:  
                    std::cout <<"\n\n Feature isn't available yet!\n";
                    break;

                    case 0:
                    std::cout <<"\n\nThank You For Using This Program....\n";
		    message += "QUIT \n"; 
                    logincheck = false;
                    useroptcheck = false;
                    break;

                    default:
                    std::cout <<"\n\nInvalid Choice!";
                    break;
            }
            if(logincheck == true){
                     //cli.SendMessage(loginMessage); 
                     //loginerror =  cli.ReceiveMessage();
		     loginerror = "OK";
                     if (loginerror == "OK"){
                         std::cout <<"Welcome!\n";                     
                         logincheck = false;
                     }
                     else if (loginerror == "ERR"){
                          if (count <= 3) 
                          {
                            std::cout <<"Wrong Username/Password!";                   
                            logincheck = true;
                            count = count + 1;
                          }
                          else if(count > 3){
                          std::cout <<"There have been 3 failed login attempts.. your account now is blocked for 10 minutes!";
                            logincheck = true;
                          }
                     }           
             }
        }

        std::vector<std::string> useroptout = {
        "\t\t---------------------------------------",
        "\t\t   .:::::::::::::::::::::::::::::::::.",
        "\t\t   .::| 1.Send mail               |::.",
        "\t\t   .::|                           |::.",
        "\t\t   .::| 2.List mail               |::.",
        "\t\t   .::|                           |::.",
        "\t\t   .::| 3.Read mail               |::.",
        "\t\t   .::|                           |::.",
        "\t\t   .::| 4.Delete mail             |::.",
        "\t\t   .::|                           |::.",
        "\t\t   .::| 0.Exit                    |::.",
        "\t\t   .:::::::::::::::::::::::::::::::::.\n",
        "\tPlease Enter Your Choice by Choosing a Number {1-4} -> ",
        };
        
	while (useroptcheck)
	{
                for(int i = 0; i < useroptout.size(); ++i) {
                        std::cout << useroptout[i];
                        std::cout << std::endl;
                }
		std::cin >> userOption;
		switch(userOption) {
		case 1 :
			std::cout << "Bitte geben Sie den Sender ein:(max. 8 chars)\n";
			message += "SEND \n";
			std::cin.ignore();
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie den Empfaenger ein:(max. 8 chars)\n";
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie den Betreff ein:(max. 80 chars)\n";
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie die Nachricht ein:(max. 920 chars)\n";
			std::getline(std::cin,buffer);
			message += buffer + "\n" + ".\n";
			std::cout << "Wollen Sie einen Anhang hinzufügen?: /home/User/...\n";
			std::getline(std::cin,fileName);
			break;
		case 2 :
			std::cout << "Bitte geben Sie den gewünschten Username ein:(max. 8 chars)\n";
			message += "LIST \n";
			std::cin.ignore();
			std::getline(std::cin,buffer);
			message += buffer + "\n" + ".\n";
			break;
		case 3 :
			std::cout << "Bitte geben Sie den gewünschten Username ein:(max. 8 chars)\n";
			message += "READ \n";
			std::cin.ignore();
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie die Nummer der Nachricht an:(max. 8 chars)\n";
			std::getline(std::cin,buffer);
			message += buffer + "\n" + ".\n";
			break;
		case 4 :
			std::cout << "Bitte geben Sie den gewünschten Username ein:(max. 8 chars)\n";
			message += "DEL \n";
			std::cin.ignore();
			std::getline(std::cin,buffer);
			message += buffer + "\n";
			std::cout << "Bitte geben Sie die Nummer der Nachricht an:(max. 8 chars)\n";
			std::getline(std::cin,buffer);
			message += buffer + "\n" + ".\n";
			break;
		case 0 :
                        std::cout <<"\n\nThank You For Using This Program....\n";
			message += "QUIT \n"; 
			useroptcheck = false;
			break;
		default:
                        std::cout <<"\n\nInvalid Choice!\n";
			break;
		}
		if(useroptcheck == true){
			cli.Connect(hostname, port);
			//std::cout << loginMessage << std::endl;
			if(fileName.length() == 0) {
				cli.SendMessage(message);
			} else {
				cli.SendMessage(message, fileName);
			}
			message.clear();
			fileName.clear();
                        cli.ReceiveMessage();
		}
	}

		return EXIT_SUCCESS;
}
