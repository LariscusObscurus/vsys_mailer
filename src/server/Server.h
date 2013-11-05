#ifndef	SERVER_H
#define SERVER_H

#include <string>
#include <vector>

class Server
{
	static const unsigned int pathLength = 256;
	static const unsigned int backLog = 10;
	static const unsigned int bufferSize = 4096;
	constexpr static const char * const attachmentDelim = "HEREBEDRAGONS!\n";
	constexpr static const char * const messageDelim = ".\n";
	constexpr static const char * const ERR = "ERR\n";
	constexpr static const char * const OK = "OK\n";

	constexpr static const char * const ldapServer = "ldap://ldap.technikum-wien.at:389";
	constexpr static const char * const ldapSearchBase = "dc=technikum-wien,dc=at";

	int m_sockfd;
	int m_childfd;
	int m_messageCount;
	bool m_loggedIn;

	std::string m_path;
	std::string m_curUserPath;
	std::string m_message;
	char *m_cbuffer;
	std::vector<char> m_bufferVector;
	std::vector<char> m_data;
	std::vector<std::string> parsedMessages;
	std::vector<std::string> m_log;

	enum MessageType {
		NONE,
		LOGIN,
		SEND,
		DEL,
		READ,
		LIST,
		ATT,
		QUIT
	}m_currentMessageType;

public:
	Server (const char *path);
	virtual ~Server ();
	/**
	 * Start:
	 * 	Server beginnt am Socket zu horchen und akzeptiert Verbindungen.
	 * 	Für jede akzeptierte Verbindung wird ein Kindprozess abgespalten.
	 */
	int Start (bool *run);
	/**
	 * Connect:
	 * 	Erstellt Socket und bindet am angegebenen port
	 */
	int Connect (const char *node, const char *port);
private:
	void ChildProcess();
	void receiveData();
	bool socketAvailable(int fd);
	bool OnRecvLOGIN();
	void OnRecvSEND();
	void OnRecvATT();
	void OnRecvDEL();
	void OnRecvREAD();
	void OnRecvLIST();
	void OnRecvQUIT();
	void sendMessage(const std::string& message);
	void createDirectory(const char *dir);
	void determineMessageType();
	bool splitMessage();
	bool splitAttached();
	/**
	 * split:
	 * 	string anhand der angegebenen Trennzeichen aufteilen
	 */
	void split(const std::string& str, const std::string& delim, std::vector<std::string>& tok);
	/**
	 * readLogFile:
	 * 	logFile neu erstellen wenn path nicht existiert
	 * 	andernfalls logFile in m_log einlesen
	 */
	void readLogFile(const std::string& path);
	void writeLogFile(const std::string& path, const std::string& subject);
	void writeMessage(const std::string& path, const std::vector<std::string>& message);
	const std::string readMessage(const std::string& path) const;
	void rewriteLog(std::string& path);
	void inputThread(bool& cont);
};
#endif
