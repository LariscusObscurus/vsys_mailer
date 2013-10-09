#ifndef	SERVER_H
#define SERVER_H

#define BACKLOG 10
#define MAX_CON 100
#define BUFFERSIZE 4096 //MACH DAS WEG

class Server
{
	int m_sockfd;
	char *m_buffer;
	struct header_t {
		char * type;
		char * sender;
		char * recipent;
		char * subject;
	} m_header;
public:
	Server ();
	virtual ~Server ();
	int Start ();
	int Connect (const char *node, const char *port);
private:
	void ChildProcess(int childfd);	
	int readHeader();
	int OnRecvSEND();
	int OnRecvDEL();
	int OnRecvREAD();
	int OnRecvLIST();
	int OnRecvQUIT();
};

#endif
