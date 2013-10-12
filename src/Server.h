#ifndef	SERVER_H
#define SERVER_H

#define PATHLENGTH 256
#define BACKLOG 10
#define MAX_CON 100
#define BUFFERSIZE 4096 //MACH DAS WEG

#include <string>
#include <vector>

class Server
{
	int m_sockfd;
	int m_childfd;
	int m_messageCount;
	std::string m_path;
	std::string m_buffer;
	std::vector<std::string> m_log;
public:
	Server (const char *path);
	virtual ~Server ();
	/**
	 * Start:
	 * 	Server beginnt am Socket zu horchen und akzeptiert verbindungen.
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
	void OnRecvSEND();
	void OnRecvDEL();
	void OnRecvREAD();
	void OnRecvLIST();
	void OnRecvQUIT();
	void sendERR();
	void createDirectory(const char * dir);
	/**
	 * split:
	 * 	string annhand der angegebenen Trennzeichen aufteilen
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
};
#endif
