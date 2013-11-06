#include "Ldap.h"
#include "Server.h"
#include "Conversion.h"
#include "ServerException.h"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <functional>

Server::Server(const char *path) :
	m_sockfd(-1),
	m_loggedIn(false),
	m_path(path),
	m_currentMessageType(NONE)
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

int Server::Start(bool *run)
{
	sockaddr_storage clientAddr;
	socklen_t sinSize;
	std::vector<char*> blackList;
	MsgBuf blockCandidate;
	key_t key = ftok("Server", 'a');
	m_msqId = msgget(key, 0600 | IPC_CREAT);

	while(*run) {
		bool updateBlacklist = true;
		do {
			errno = 0;
			if(msgrcv(m_msqId, &blockCandidate, sizeof(MsgBuf) - sizeof(long), 0, IPC_NOWAIT) != -1) {
				blackList.push_back(blockCandidate.blacklisted);
			} else if(errno == ENOMSG) {
				updateBlacklist = false;
			} else {
				std::cout << std::strerror(errno) << std::endl;
			}
		} while(updateBlacklist);

		sinSize = sizeof(clientAddr);
		if(!socketAvailable(m_sockfd)) {
			usleep(500);
			continue;
		}
		m_childfd = accept(m_sockfd,(sockaddr *)&clientAddr, &sinSize);
		if(m_childfd == -1) {
			continue;
		}
		inet_ntop(clientAddr.ss_family, get_in_addr((struct sockaddr*)&clientAddr),m_clientIp, sizeof(m_clientIp));
		if(!fork()) {
			close(m_sockfd);
			std::cout << m_clientIp << std::endl;
			ChildProcess();
			close(m_childfd);
			return 0;
		}
		close(m_childfd);
	}
	return 0;
}

void Server::ChildProcess()
{
	int i;
	try {
		while(true) {
			receiveData();
			determineMessageType();
			if(m_currentMessageType != ATT) {
				for(i = 0; i <= 5000; i++) {	// 5000 * bufferSize Limit ~20MB
					if(splitMessage()) {
						break;
					}
					receiveData();
				}
			} else {
				for(i = 0; i <= 5000; i++) {
					if(splitAttached()) {
						break;
					}
					receiveData();
				}

			}
			if(i == 5000) { throw ServerException("Message size exceeded"); }

			switch (m_currentMessageType) {
			case LOGIN:
				for(int i = 0; i <= 2; i++) {
					m_loggedIn = OnRecvLOGIN();
				}
				break;
			case SEND:
				OnRecvSEND();
				break;
			case ATT:
				OnRecvATT();
				break;
			case READ:
				OnRecvREAD();
				break;
			case DEL:
				OnRecvDEL();
				break;
			case LIST:
				OnRecvLIST();
				break;
			case QUIT:
				return;
			default:
				sendMessage(ERR);
				break;
			}
			m_message.clear();
		}
	}
	catch(const ServerException& ex) {
		sendMessage(ERR);
		std::cout << ex.what() << std::endl;
	}
}

void Server::receiveData()
{
	long bytesReceived = 0;
	errno = 0;

	memset(m_cbuffer, 0, bufferSize);

	if((bytesReceived = recv(m_childfd, m_cbuffer, bufferSize, 0)) != -1) {
		std::copy(m_cbuffer, m_cbuffer + bytesReceived, std::back_inserter<std::vector<char>>(m_buffer));
	}

	switch(errno) {
	case 0:
		return;
	default:
		throw ServerException(std::strerror(errno));
	}
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

void Server::determineMessageType()
{
	if(m_buffer.empty()) {return;}
	std::string header(m_buffer.begin(), m_buffer.begin() + 5);
	if(!header.substr(0,5).compare("LOGIN")) {
		m_currentMessageType = LOGIN;
		std::cout << "LOGIN RECEIVED" << std::endl;
	} else if(!header.substr(0,4).compare("SEND")) {
		m_currentMessageType = SEND;
		std::cout << "SEND RECEIVED" << std::endl;
	} else if(!header.substr(0,4).compare("READ")) {
		m_currentMessageType = READ;
	} else if(!header.substr(0,4).compare("LIST")) {
		m_currentMessageType = LIST;
	} else if(!header.substr(0,4).compare("QUIT")) {
		m_currentMessageType = QUIT;
	} else if(!header.substr(0,3).compare("DEL")) {
		m_currentMessageType = DEL;
	} else if(!header.substr(0,3).compare("ATT")) {
		m_currentMessageType = ATT;
	} else {
		sendMessage(ERR);
		std::cout << header << std::endl;
		m_buffer.clear();
		throw ServerException("Received invalid Message");
	}
}

bool Server::splitMessage()
{
	size_t delimSize = strlen(messageDelim);
	auto delimPos = std::search(m_buffer.begin(), m_buffer.end(), messageDelim, messageDelim + delimSize);
		if(delimPos != m_buffer.end()) {
			m_message = std::string(m_buffer.begin(), delimPos + delimSize);
			std::cout << m_message << std::endl;
			m_buffer.erase(m_buffer.begin(),delimPos + delimSize);
			return true;
		}

	return false;
}

bool Server::splitAttached()
{

	size_t delimSize = strlen(attachmentDelim);
	auto delimPos = std::search(m_buffer.begin(), m_buffer.end(), attachmentDelim, attachmentDelim + delimSize);
	if(delimPos != m_buffer.end()) {
		std::cout << "Ende gefunden" << std::endl;
		std::move(m_buffer.begin() + strlen("ATT"), m_buffer.end() - delimSize, std::back_inserter(m_data));
		m_buffer.erase(m_buffer.begin(),delimPos + delimSize);
		return true;
	}
	return false;
}

bool Server::OnRecvLOGIN()
{
	std::vector<std::string> lines;
	split(m_message, "\n", lines);
	try {
		Ldap ldapConnection(static_cast<const char * const>(ldapServer));
		ldapConnection.bind("uid=if12b076,ou=people,dc=technikum-wien,dc=at", "");
		if(ldapConnection.authenticate(lines[1], lines[2], ldapSearchBase)) {
			sendMessage(OK);
			return true;
		}
		return false;
	} catch(const ServerException& ex) {
		std::cout << ex.what() << std::endl;
	}
	sendMessage(ERR);
	return false;
}

void Server::OnRecvSEND()
{
	if(!m_loggedIn) {return;}
	std::vector<std::string> lines;

	split(m_message, "\n", lines);

	/**
	 * USER directory erstellen
	 */
	m_curUserPath = std::string(m_path + "/" + lines[2] + "/");
	try {
		createDirectory(m_curUserPath.c_str());
	} catch(const ServerException& ex) {
		std::cerr << ex.what() << std::endl;
	}

	std::string logFile(m_curUserPath + "log");
	readLogFile(logFile);
	writeMessage(m_curUserPath + numberToString<int>(++m_messageCount), lines);
	writeLogFile(logFile, lines[3]);

	return;
}

void Server::OnRecvATT()
{
	std::ofstream dataStream(m_curUserPath + numberToString<int>(m_messageCount) + "_data",
		std::ios::out|std::ios::binary|std::ios::trunc);

	if(!dataStream.is_open()) {
		throw ServerException("writeData: Could not open file");
	}
	try {
		dataStream.write(reinterpret_cast<char*>(&m_data[0]), m_data.size());
	} catch(const std::fstream::failure& ex) {
		std::cout << ex.what() << std::endl;
		throw ServerException("writeData: Could not write file");
	}
	return;
}

void Server::OnRecvDEL()
{
	if(!m_loggedIn) {return;}
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
	if(!m_loggedIn) {return;}
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
	if(!m_loggedIn) {return;}

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
		throw ServerException("writeMessage: Could not open File");
	}
	for(auto& it: message) {
		messageStream << it << "\n";
	}
	messageStream.close();
}

const std::string Server::readMessage(const std::string& path) const
{
	std::string message;
	std::fstream messageStream(path,std::ios::in);
	if(!messageStream.is_open()) {
		throw ServerException("readMessage: Could not open file");
	} else {
		message = std::string (std::istreambuf_iterator<char>(messageStream),
			  std::istreambuf_iterator<char>());
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

void *Server::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return nullptr;
}
