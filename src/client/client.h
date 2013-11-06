#ifndef	CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>

class Client
{
	int m_sockfd;
	static const int bufferLength = 4096;
	constexpr static const char * messageDelim = ".\n";
	std::vector<char> m_buffer;
	std::string m_message;
public:
	Client();
	virtual ~Client();
	int Connect (char* const server_hostname, const char* server_service);
	int SendMessage(std::string message);
	int SendMessage(std::string message, std::string fileName);
	int Login(const char *user, const char *p);

	bool splitMessage();
	void receiveData();
	void receiveData(int amount);
	/**
	 * checkOK:
	 * 	Überprüft ob der Server OK oder ERR genantwortet hat.
	 */
	int checkOK();
	void printMessage();
private:
	int ReadFile(std::string fileName, std::vector<unsigned char> &out);
};

#endif
