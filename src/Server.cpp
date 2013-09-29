#include "Server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>

Server::Server() : 
	m_sockfd(-1)
{

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
			close(m_sockfd);
			if(send(childfd, "Hello, world!", 13, 0) == -1) {
				/* Fehler */
			}
			close(childfd);
			exit(0);
		}
		close(childfd);
	}
	return 0;
}
