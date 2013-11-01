#ifndef SERVEREXCEPTION_H
#define SERVEREXCEPTION_H

#include <stdexcept>

class ServerException : public std::exception
{
	const char* m_msg;
public:
	ServerException(const char *msg);
	virtual ~ServerException() throw();
	virtual const char* what() const throw();
};

#endif // SERVEREXCEPTION_H
