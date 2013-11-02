#include "Ldap.h"
#include "Server.h"
#include "Conversion.h"
#include "ServerException.h"
#include <sys/types.h>
#include <sys/time.h>
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
#include <thread>
#include <functional>

Server::Server(const char *path) :
	m_sockfd(-1),
	m_path(path)
{
	m_cbuffer = new char[bufferSize];
}

Server::~Server()
{
	delete[] m_cbuffer;
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
	bool cont = true;

	std::thread t1(&Server::inputThread,this,std::ref(cont));

	while(cont) {
		sinSize = sizeof(clientAddr);
		if(!socketAvailable(m_sockfd)) {
			usleep(500);
			continue;
		}
		m_childfd = accept(m_sockfd,(sockaddr *)&clientAddr, &sinSize);
		if(m_childfd == -1) {
			continue;
		}
		if(!fork()) {
			close(m_sockfd);
			ChildProcess();
			close(m_childfd);
			_exit(0);
		}
		close(m_childfd);
	}
	t1.join();
	return 0;
}

void Server::ChildProcess()
{
	try {
		if(!receiveLogin()) {
			return;
		}
		sendMessage(OK);
		receiveData();
	} catch(const ServerException& ex) {
		return;
	}


	splitAttached(m_buffer, attachmentDelim);
#ifdef _DEBUG
	std::cout << "Message: " << m_message << std::endl;
	std::cout << "Attachment Size: " << m_data.size() << std::endl;
	for(auto& it: m_data) {
		std::cout << it;
	}
	std::cout<< std::endl;
#endif
	try {
		if(!strncmp("SEND", m_message.c_str(), 4)){
			OnRecvSEND();
		} else if(!strncmp("READ", m_message.c_str(), 4)) {
			OnRecvREAD();
		} else if(!strncmp("LIST", m_message.c_str(), 4)) {
			OnRecvLIST();
		} else if(!strncmp("QUIT", m_message.c_str(), 4)) {
			OnRecvQUIT();
		} else if(!strncmp("DEL", m_message.c_str(), 3)) {
			OnRecvDEL();
		} else {
			sendMessage(ERR);
		}
	} catch(const ServerException& ex) {
		sendMessage(ERR);
		std::cout << ex.what() << std::endl;
	}
}

bool Server::receiveLogin()
{
	bool result = false;
	for(int i = 3; i <= 3; i++) {
		receiveData();
		m_message = std::string(m_buffer.begin(), m_buffer.end());
		if(m_message.substr(0,4).compare("LOGIN")) {
			std::cout << m_message;
			result = OnRecvLOGIN();
		}
		if(result == false) {
			sendMessage(ERR);
		} else {
			return result;
		}
	}
	return result;
}

void Server::receiveData()
{
	long bytesReceived = 0;

	m_buffer.clear();
	for(int i = 2; i <= 2; i++) {
		do {
			errno = 0;
			if(!socketAvailable(m_childfd)) {
				break;
			}
			if((bytesReceived = recv(m_childfd, m_cbuffer, bufferSize, 0)) != -1) {
				std::copy(m_cbuffer, m_cbuffer + bytesReceived, std::back_inserter<std::vector<char>>(m_buffer));
			}
			switch(errno) {
			case 0:
				break;
			case EAGAIN:
				bytesReceived = 0;
				break;
			default:
				sendMessage(ERR);
				throw ServerException(std::strerror(errno));
				return;
			}
			memset(m_cbuffer, 0, bufferSize);
		} while(bytesReceived >= (int)bufferSize -1);

		if(m_buffer.size() == 0) {
			sleep(3);
			continue;
		} else {
			return;
		}
	}
	sendMessage(ERR);
	throw ServerException("Connection timed out");
}

bool Server::socketAvailable(int fd)
{
	bool result;
	fd_set sready;
	struct timeval nowait;

	FD_ZERO(&sready);
	FD_SET((unsigned int)fd, &sready);
	memset((char *)&nowait,0,sizeof(nowait));

	result = select(fd + 1, &sready, nullptr, nullptr, &nowait);
	if( FD_ISSET(fd, &sready) ) {
		result = true;
	} else {
		result = false;
	}

	return result;
}

bool Server::OnRecvLOGIN()
{
	std::vector<std::string> lines;
	split(m_message, "\n", lines);

	try {
		Ldap ldapConnection(static_cast<const char * const>(ldapServer));
		ldapConnection.bind("uid=if12b076,ou=people,dc=technikum-wien,dc=at", "");
		return ldapConnection.authenticate(lines[1], lines[2], ldapSearchBase);
	} catch(const ServerException& ex) {
		std::cout << ex.what() << std::endl;
	}
	sendMessage(ERR);
	return false;
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
	} catch(const ServerException& ex) {
		std::cerr << ex.what() << std::endl;
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
	sendMessage(ERR);
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
	} catch(const ServerException& ex) {
		sendMessage(ERR);
		std::cout << ex.what() << std::endl;
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

void Server::sendMessage(const std::string& message)
{
	send(m_childfd, message.c_str(), message.length(), 0);
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
	std::move(splitLine + delim.length(), buffer.end(), std::back_inserter(m_data));
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
		logString = std::string (std::istreambuf_iterator<char>(logFileStream),
			  std::istreambuf_iterator<char>());
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

void Server::inputThread(bool& cont)
{
	std::string tmp;
	std::getline(std::cin,tmp);
	cont = false;
}
