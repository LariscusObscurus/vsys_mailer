#ifndef	SERVER_H
#define SERVER_H

#define BACKLOG 10

class Server
{
	int m_sockfd;

public:
	Server ();
	virtual ~Server ();
	int Start ();
	int Connect (const char *node, const char *port);
private:
	
};

#endif
