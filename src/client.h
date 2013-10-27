#ifndef	CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>

class Client
{
	int m_sockfd;
public:
	Client();
	virtual ~Client();
	int Connect (char* const server_hostname, const char* server_service);
	int SendMessage(const char *message, int size);
};

#endif
