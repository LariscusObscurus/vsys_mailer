#include "Server.h"
#include "Conversion.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <fstream>


Server::Server(const char *path) : 
	m_sockfd(-1),
	m_path(path)
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

	createDirectory(m_path.c_str());

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
	char *buf = new char[BUFFERSIZE];

	/**
	 * TODO:
	 * Buffergröße absichern
	 */
	const char * msg = "OK\n";

	if(send(childfd, msg,strlen(msg), 0) == -1) {
		delete[] buf;
		return;
	}
	if((bytesReceived = recv(childfd,buf ,BUFFERSIZE, 0)) == -1) {
		delete[] buf;
		sendERR(childfd);
		return;
	}
	buf[bytesReceived] = '\0';
	/**
	 * FIXME:
	 * Message größer als BUFFERSIZE wird abgeschnitten
	 */
	std::cout << "buffer:" << std::endl << buf << std::endl;
	m_buffer = std::string(buf);

	try {
		if(!strncmp("SEND", buf, 4)){
			OnRecvSEND();
		} else if(!strncmp("READ", buf, 4)) {
			OnRecvREAD();
		} else if(!strncmp("LIST", buf, 4)) {
			OnRecvLIST();
		} else if(!strncmp("QUIT", buf, 4)) {
			OnRecvQUIT();
		} else if(!strncmp("DEL", buf, 4)) {
			OnRecvDEL();
		} else {
			sendERR(childfd);
		}
	} catch(const char* ex) {
		sendERR(childfd);
	}
	delete[] buf;
	close(childfd);
}

void Server::OnRecvSEND()
{
	std::string tmp;
	int messageCount;
	std::string logString;
	std::string delim ("\n");
	std::vector<std::string> lines;

	split(m_buffer, delim, lines);
	
	/**
	 * USER directory erstellen
	 */
	std::string dir(realpath(m_path.c_str(), NULL));
	dir.append("/" + lines[2]);
	createDirectory(dir.c_str());

	std::cout << "Split:" << std::endl;
	for(auto& it: lines) {
		std::cout << it << std::endl;
	}

	/**
	 * logFile erstellen
	 * TODO: 
	 * In Methode auslagern.
	 */
	std::string fileName(dir + "/log");

	std::fstream logFileStream;
	logFileStream.open(fileName,std::ios::in|std::ios::app);

	if(!logFileStream.is_open()) {
		logFileStream.clear();
		logFileStream.open(fileName, std::ios::out);
		logFileStream.close();
		logFileStream.open(fileName, std::ios::in|std::ios::app);
		messageCount = 0;
	} else {
		messageCount = 0;
		while(logFileStream.good()) {
			/**
			 * FIXME:
			 * Whitespaces am Ende von logString entfernen
			 */
			char buf[100] = {};
			logFileStream.read(&buf[0],100);
			logString.append(buf);
		}
		std::cout << logString << std::endl;
	}
	std::cout << messageCount << std::endl;
	logFileStream.close();
	return;
}

void Server::OnRecvDEL()
{
	return;	
}
void Server::OnRecvREAD()
{
	return;	
}
void Server::OnRecvLIST()
{
	return;	
}
void Server::OnRecvQUIT()
{
	return;	
}

void Server::sendERR(int childfd)
{
	const char * err = "ERR\n";

	send(childfd, err, strlen(err), 0);
	return;
	
}
void Server::createDirectory(const char * dir)
{
	struct stat st = {};
	if(stat(dir, &st) == -1) {
		if(( mkdir(dir, 0700)) == -1) {
			throw (const char *) std::strerror(errno);
		}
	}

}

void Server::split(const std::string& str, const std::string& delim, std::vector<std::string>& tokens)
{
	std::string::size_type lastPos = str.find_first_not_of(delim,0);
	std::string::size_type pos = str.find_first_of(delim, lastPos);

	while(std::string::npos != pos || std::string::npos != lastPos) {
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(delim, pos);
		pos = str.find_first_of(delim, lastPos);
	}
}
