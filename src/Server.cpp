#include "Server.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>

Server::Server(const char *path) : 
	m_sockfd(-1)
{	
	m_path = path;
	struct stat st = {};
	if(stat(m_path, &st) == -1) {
		mkdir(m_path, 0700);
	}
}

Server::~Server()
{
	close(m_sockfd);
}

int Server::Connect (const char *node, const char *port)
{
	int rv;
	int optval = 1;
	addrinfo *servinfo, *p, hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(node, port, &hints, &servinfo) != 0)) {
		throw gai_strerror(rv);
	}
	
	for(p = servinfo; p != NULL; p = p->ai_next) {

		if((m_sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
			servinfo->ai_protocol)) == -1) {
			continue;
		}
		if(setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
			throw "Server: setsockopt";
		}

		if(bind(m_sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) {
			close(m_sockfd);
			continue;
		}
		break;
	}

	if(p == NULL)
	{
		throw "Server: Could not create socket";

	}
	freeaddrinfo(servinfo);

	if(listen(m_sockfd, BACKLOG) == -1) {
		throw std::strerror(errno);
	}
	return 0;
}

int Server::Start()
{
	sockaddr_storage clientAddr;
	socklen_t sinSize;
	int childfd;

	while(1) {
		sinSize = sizeof(clientAddr);
		childfd = accept(m_sockfd,(sockaddr *)&clientAddr, &sinSize);
		if(childfd == -1) {
			continue;
		}
		if(!fork()) {
			ChildProcess(childfd);
			delete[] m_buffer;
			close(childfd);
			exit(0);
		}
		close(childfd);
	}
	return 0;
}

void Server::ChildProcess(int childfd)
{
	close(m_sockfd);
	long bytesReceived;

	m_buffer = new char[BUFFERSIZE];
	/**
	 * TODO:
	 * Buffergröße absichern
	 */
	const char * msg = "OK\n";
	const char * err = "ERR\n";

	if(send(childfd, msg,strlen(msg), 0) == -1) {
		return;
	}
	if((bytesReceived = recv(childfd,m_buffer ,BUFFERSIZE, 0)) == -1) {
		send(childfd, err, strlen(err), 0);
		return;
	}
	m_buffer[bytesReceived] = '\0';

	std::cout << m_buffer << std::endl;
	splitMessage();
	std::cout << m_header.type << std::endl;
	std::cout << m_header.sender << std::endl;

	if(!strncmp("SEND", m_header.type, 4)){
		OnRecvSEND();
	} else if(!strncmp("READ", m_header.type, 4)) {
		OnRecvREAD();
	} else if(!strncmp("LIST", m_header.type, 4)) {
		OnRecvLIST();
	} else if(!strncmp("QUIT", m_header.type, 4)) {
		OnRecvQUIT();
	} else if(!strncmp("DEL", m_header.type, 4)) {
		OnRecvDEL();
	} else {
		send(childfd, err, strlen(err), 0);
	}
	close(childfd);
}

int Server::OnRecvSEND()
{
	char * dir = new char[PATHLENGTH];
	strcpy(dir, realpath(m_path, NULL));
	strcat(dir, "/");
	strcat(dir, m_header.sender);

	struct stat st = {};
	if(stat(dir, &st) == -1) {
		mkdir(dir, 0700);
	}
	
	
	
	return 0;
}

int Server::OnRecvDEL()
{
	return 0;	
}
int Server::OnRecvREAD()
{
	return 0;	
}
int Server::OnRecvLIST()
{
	return 0;	
}
int Server::OnRecvQUIT()
{
	return 0;	
}

int Server::splitMessage()
{
	int i;
	char * token, *save, *str;
	for(i = 1, str = m_buffer;i <= 4; i++, str = NULL) {
		token = strtok_r(str, "\n", &save);
		std::cout << token << std::endl;
		if(token == NULL) {
			break;
		}

		switch(i) {
		case 1:
			m_header.type = token;
			break;
		case 2:
			m_header.sender = token;
			break;
		case 3:
			m_header.recipent = token;
			break;
		case 4:
			m_header.subject = token;
			break;
		default:
			strcat(m_message, token);
		}
			
	}		
	return 0;
}
