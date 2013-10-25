#include <ldap.h>
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
#include <algorithm>
#include <iterator>

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

	/*realpath must be free'd*/
	char * tmp = realpath(m_path.c_str(), nullptr);
	m_path = tmp;
	free(tmp);

	if((rv = getaddrinfo(node, port, &hints, &servinfo) != 0)) {
		throw ServerException(gai_strerror(rv));
	}

	for(p = servinfo; p != nullptr; p = p->ai_next) {

		if((m_sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
			servinfo->ai_protocol)) == -1) {
			continue;
		}
		if(setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
			throw ServerException("Server: setsockopt");
		}

		if(bind(m_sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) {
			close(m_sockfd);
			continue;
		}
		break;
	}

	if(p == nullptr)
	{
		throw ServerException("Server: Could not create socket");

	}
	freeaddrinfo(servinfo);

	if(listen(m_sockfd, backLog) == -1) {
		throw ServerException(std::strerror(errno));
	}
	return 0;
}

int Server::Start()
{
	sockaddr_storage clientAddr;
	socklen_t sinSize;

	while(1) {
		sinSize = sizeof(clientAddr);
		m_childfd = accept(m_sockfd,(sockaddr *)&clientAddr, &sinSize);
		if(m_childfd == -1) {
			continue;
		}
		if(!fork()) {
			close(m_sockfd);
			ChildProcess();
			close(m_childfd);
			exit(0);
		}
		close(m_childfd);
	}
	return 0;
}

void Server::ChildProcess()
{
	long bytesReceived;
	char *buf = new char[bufferSize];
	std::vector<char> bufferVector;

	const char * msg = "OK\n";

	if(send(m_childfd, msg,strlen(msg), 0) == -1) {
		delete[] buf;
		return;
	}
	do {
		errno = 0;
		if((bytesReceived = recv(m_childfd,buf, bufferSize, MSG_DONTWAIT)) != -1) {
			std::copy(buf, buf + bytesReceived, std::back_inserter<std::vector<char>>(bufferVector));
		}
		switch(errno) {
		case 0:
			break;
		case EAGAIN:
			bytesReceived = 0;
			break;
		default:
			delete[] buf;
			sendERR();
			return;
		}
		memset(buf, 0, bufferSize);
	} while(bytesReceived >= bufferSize -1);
	delete[] buf;

	splitAttached(bufferVector,attachmentDelim);

	try {
		if(!strncmp("SEND", m_message.c_str(), 4)){
			OnRecvSEND();
		} else if(!strncmp("LOGIN", m_message.c_str(), 4)) {
			OnRecvLOGIN();
		} else if(!strncmp("READ", m_message.c_str(), 4)) {
			OnRecvREAD();
		} else if(!strncmp("LIST", m_message.c_str(), 4)) {
			OnRecvLIST();
		} else if(!strncmp("QUIT", m_message.c_str(), 4)) {
			OnRecvQUIT();
		} else if(!strncmp("DEL", m_message.c_str(), 3)) {
			OnRecvDEL();
		} else {
			sendERR();
		}
	} catch(ServerException) {
		sendERR();
	}
}

void Server::OnRecvLOGIN()
{
	std::vector<std::string> lines;
	split(m_message, "\n", lines);
	LDAP *ld;
	if((ld =ldap_open(ldapServer, LDAP_PORT)) == nullptr) {
		sendERR();
		return;
	}
	std::string dn("uid=" + lines[1] +", ou=people, dc=technikum-wien, dc=at");
	if(ldap_bind_s(ld, dn.c_str(), lines[2].c_str(), LDAP_AUTH_SIMPLE) != LDAP_SUCCESS) {
		sendERR();
		return;
	}
	std::cout << "SUCCESSFULL LOGIN" << std::endl;
	ldap_unbind(ld);
}

void Server::OnRecvSEND()
{
	std::vector<std::string> lines;

	split(m_message, "\n", lines);

	/**
	 * USER directory erstellen
	 */
	std::string dir(m_path + "/" + lines[2] + "/");
	try {
	createDirectory(dir.c_str());
	} catch(ServerException e) {
		std::cerr << e.what() << std::endl;
	}

	std::string logFile(dir + "log");
	readLogFile(logFile);
	writeMessage(dir + numberToString<int>(++m_messageCount), lines);
	writeLogFile(logFile, lines[3]);

	return;
}

void Server::OnRecvDEL()
{
	std::string msg;
	std::vector<std::string> lines;
	split(m_message, "\n", lines);

	std::string dir(m_path + "/" + lines[1] + "/" + lines[2]);
	std::string logDir(m_path + "/" + lines[1] + "/log");

	readLogFile(logDir);
	for(int i = 0; i <= (int)m_log.size(); i++) {
		std::string::size_type pos = m_log[i].find_first_of(";",0);
		if(m_log[i].substr(0,pos) == lines[2]){
			m_log.erase(m_log.begin() + i);
			rewriteLog(logDir);
			unlink(dir.c_str());
			return;
		}
	}
	sendERR();
	return;
}

void Server::OnRecvREAD()
{
	std::string msg;
	std::vector<std::string> lines;
	split(m_message, "\n", lines);

	std::string dir(m_path + "/" + lines[1] + "/" + lines[2]);

	try {
		msg = readMessage(dir);
	} catch(ServerException) {
		sendERR();
	}
	send(m_childfd, msg.c_str(), msg.size(), 0);
	return;
}

void Server::OnRecvLIST()
{
	std::string msg;
	std::vector<std::string> lines;
	split(m_message, "\n", lines);

	std::string dir(m_path + "/" + lines[1] + "/log");
	readLogFile(dir);
	msg += numberToString<int>(m_messageCount) + "\n";
	for(auto it: m_log) {
		std::vector<std::string> tmp;
		split(it, ";", tmp);
		msg += tmp[1] + "\n";
	}
	send(m_childfd, msg.c_str(), msg.size(), 0);
	return;
}
void Server::OnRecvQUIT()
{
	return;
}

void Server::sendERR()
{
	const char * err = "ERR\n";

	send(m_childfd, err, strlen(err), 0);
	return;

}

void Server::createDirectory(const char * dir)
{
	struct stat st = {};
	if(stat(dir, &st) == -1) {
		if(( mkdir(dir, 0700)) == -1) {
			throw ServerException((const char *) std::strerror(errno));
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

void Server::splitAttached(const std::vector<char> &buffer, const std::string &delim)
{
	auto splitLine = std::search(buffer.begin(), buffer.end(), delim.begin(), delim.end());
	if(splitLine == buffer.end()) {
		m_message = std::string(buffer.begin(), buffer.end());
		return;
	}
	m_message = std::string(buffer.begin(), splitLine);
	std::move(splitLine, buffer.end(), std::back_inserter(m_data));
}

void Server::readLogFile(const std::string& path)
{
	std::string logString;
	std::fstream logFileStream;
	logFileStream.open(path,std::ios::in);

	if(!logFileStream.is_open()) {
		logFileStream.clear();
		logFileStream.open(path, std::ios::out);
		logFileStream.close();
		m_messageCount = 0;
		return;
	} else {
		while(logFileStream.good()) {
			/**
			 * FIXME:
			 * Whitespaces am Ende von logString entfernen
			 */
			char buf[100] = {};
			logFileStream.read(&buf[0],100);
			logString += buf;
		}
	}
	logFileStream.close();
	if(logString.size() > 0) {
		split(logString,"\n", m_log);
		std::string lastLine(m_log[m_log.size() - 1]);
		std::string number(lastLine.substr(0, lastLine.find_first_of(";")));
		m_messageCount = stringToNumber<int>(number);

	} else {
		m_messageCount = 0;
	}

#ifdef _DEBUG
	std::cout << "Log:" << std::endl;
	for(auto& it: m_log) {
		std::cout << it << std::endl;
	}
	std::cout << m_messageCount << std::endl;
#endif
}

void Server::writeLogFile(const std::string& path, const std::string& subject)
{
	std::fstream logStream(path, std::ios::out|std::ios::app);
	if(!logStream.is_open()) {
		throw ServerException("writeLogFile");
	}
	logStream << numberToString<int>(m_messageCount)
		<< ";" << subject
		<< "\n";

	logStream.close();
}

void Server::writeMessage(const std::string& path, const std::vector<std::string>& message)
{
	std::fstream messageStream(path, std::ios::out|std::ios::trunc);
	if(!messageStream.is_open()) {
		throw ServerException("writeMessage");
	}
	for(auto it: message) {
		messageStream << it << "\n";
	}
	messageStream.close();
}

const std::string Server::readMessage(const std::string& path) const
{
	std::string message;
	std::fstream messageStream(path,std::ios::in);
	if(!messageStream.is_open()) {
		throw ServerException("readMessage");
	}
	while(messageStream.good()) {
		char buf[100] = {};
		messageStream.read(&buf[0],100);
		message += buf;
	}
	messageStream.close();
	return message;
}

void Server::rewriteLog(std::string& path) {
	std::fstream logStream(path, std::ios::out|std::ios::trunc);
	if(!logStream.is_open()) {
		throw ServerException("rewriteLog");
	}
	for(auto &it: m_log) {
		logStream << it << "\n";
	}
	logStream.close();
}

Server::ServerException::ServerException(const char*msg) :
	m_msg(msg)
{

}

Server::ServerException::~ServerException() throw()
{

}

const char * Server::ServerException::what() const throw()
{
	return m_msg;
}
