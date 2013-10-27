#ifndef	SERVER_H
#define SERVER_H

#include <stdexcept>
#include <string>
#include <vector>

class Server
{
	static const unsigned int pathLength = 256;
	static const unsigned int backLog = 10;
	static const unsigned int bufferSize = 4096;
	constexpr static const char * attachmentDelim = "HEREBEDRAGONS!";

	constexpr static const char * ldapServer = "ldap.technikum-wien.at";
	constexpr static const char * ldapSearchBase = "dc=technikum-wien.at,dc=at";
	constexpr static const char * ldapFilter = "(uid=if09b*)";

	int m_sockfd;
	int m_childfd;
	int m_messageCount;
	std::string m_path;
	std::string m_message;
	std::vector<char> m_data;
	std::vector<std::string> m_log;

public:
	class ServerException : public std::exception
	{
		const char* m_msg;
		public:
			ServerException(const char *msg);
			virtual ~ServerException() throw();
			virtual const char* what() const throw();
	};

	Server (const char *path);
	virtual ~Server ();
	/**
	 * Start:
	 * 	Server beginnt am Socket zu horchen und akzeptiert Verbindungen.
	 * 	Für jede akzeptierte Verbindung wird ein Kindprozess abgespalten.
	 */
	int Start ();
	/**
	 * Connect:
	 * 	Erstellt Socket und bindet am angegebenen port
	 */
	int Connect (const char *node, const char *port);
private:
	void ChildProcess();
	void OnRecvLOGIN();
	void OnRecvSEND();
	void OnRecvDEL();
	void OnRecvREAD();
	void OnRecvLIST();
	void OnRecvQUIT();
	void sendERR();
	void createDirectory(const char * dir);
	void splitAttached(const std::vector<char>& buffer, const std::string& delim);
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
};
#endif
