#ifndef	SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <string>
#include <vector>

class Server
{
	/**
	 * Konstanten
	 */
	static const unsigned int pathLength = 256;
	static const unsigned int backLog = 10;
	static const unsigned int bufferSize = 4096;
	static const unsigned int timeBanned = 30;
	constexpr static const char * const messageDelim = ".\n";
	constexpr static const char * const ERR = "ERR\n.\n";
	constexpr static const char * const OK = "OK\n.\n";

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
	char m_clientIp[INET6_ADDRSTRLEN];
	std::vector<char> m_buffer;
	std::vector<std::string> parsedMessages;
	std::vector<std::string> m_log;

	/**
	 * Aktuell zu bearbeitender Nachrichtentypus
	 */
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

	/**
	 * Blacklist
	 */
	struct MsgBuf {
		long mtype;
		char blacklisted[INET6_ADDRSTRLEN];
	};
	struct BlackListEntry {
		time_t time;
		char blacklisted[INET6_ADDRSTRLEN];
	};

	key_t m_key;

public:
	Server (const char *path);
	virtual ~Server ();
	/**
	 * Connect:
	 * 	Erstellt Socket und bindet am angegebenen port
	 */
	int Connect (const char *node, const char *port);
	/**
	 * Start:
	 * 	Server beginnt am Socket zu horchen und akzeptiert Verbindungen.
	 * 	Für jede akzeptierte Verbindung wird ein Kindprozess abgespalten.
	 */
	int Start (bool *run);
private:
	/**
	 * get_in_addr:
	 * 	Socket Addresse für ip4 und ip6
	 */
	void *get_in_addr(struct sockaddr *sa);
	/**
	 * ChildProcess:
	 * 	Dies ist die Main Funktion des Kindprozesses.
	 */ 	
	void ChildProcess();
	void receiveData();
	void sendMessage(const std::string& message);
	bool socketAvailable(int fd);
	void determineMessageType();
	/**
	 * splitMessage:
	 * 	spaltet eine Nachricht in m_buffer anhand von messageDelim ab.
	 */
	bool splitMessage();
	/**
	 * blacklistSend:
	 * 	Sollte ein Client sich dreimal falsch einloggen sendet diese
	 * 	Methode die IP an den Parent Prozess.
	 */
	void blacklistSend();
	/**
	 * OnRecv* Methoden:
	 * 	Die folgenden Methoden sind dafür Zuständig die entsprechenden
	 * 	Nachrichten zu verarbeiten.
	 */
	bool OnRecvLOGIN();
	void OnRecvSEND();
	void OnRecvATT();
	void OnRecvDEL();
	void OnRecvREAD();
	void OnRecvLIST();
	void OnRecvQUIT();

	/**
	 * split:
	 * 	string anhand der angegebenen Trennzeichen aufteilen
	 */
	void split(const std::string& str, const std::string& delim, std::vector<std::string>& tok);
	void createDirectory(const char *dir);
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
