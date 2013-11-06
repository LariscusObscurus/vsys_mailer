#ifndef	CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>

class Client
{
	int m_sockfd;
	static const int bufferLength = 4096;
	constexpr static const char * attachmentDelim = "HEREBEDRAGONS!\n";

public:
	Client();
	virtual ~Client();
	int Connect (char* const server_hostname, const char* server_service);
        char ReceiveMessage();
	int SendMessage(std::string message);
	int SendMessage(std::string message, std::string fileName);
	int Login(const char *user, const char *p);
private:
	int ReadFile(std::string fileName, std::vector<unsigned char> &out);
};

#endif
