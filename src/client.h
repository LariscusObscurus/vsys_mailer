#ifndef	CLIENT_H
#define CLIENT_H

#define BACKLOG 10
#define MAX_CON 100
#define BUFFERSIZE 4096 //MACH DAS WEG

#include <string>
#include <vector>

class Client
{
	int m_sockfd;
	std::string m_buffer;
	std::string m_message;
public:
	Client();
	virtual ~Client();
	int Connect (char* const server_hostname, const char* server_service);
	int SendMessage(char* const message, int size);
};

#endif
