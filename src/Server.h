#ifndef	SERVER_H
#define SERVER_H

#define PATHLENGTH 256
#define BACKLOG 10
#define MAX_CON 100
#define BUFFERSIZE 4096 //MACH DAS WEG

#include <string>
#include <vector>

class Server
{
	int m_sockfd;
	std::string m_path;
	std::string m_buffer;
	std::string m_message;
public:
	Server (const char *path);
	virtual ~Server ();
	int Start ();
	int Connect (const char *node, const char *port);
private:
	void ChildProcess(int childfd);	
	void OnRecvSEND();
	void OnRecvDEL();
	void OnRecvREAD();
	void OnRecvLIST();
	void OnRecvQUIT();
	void sendERR(int childfd);
	void createDirectory(const char * dir);
	void split(const std::string& str, const std::string& delim, std::vector<std::string>& tok);
};

#endif
