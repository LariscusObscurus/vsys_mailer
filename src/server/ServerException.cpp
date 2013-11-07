#include "ServerException.h"

ServerException::ServerException(const char*msg) :
	m_msg(msg)
{

}

ServerException::~ServerException() throw()
{

}

const char * ServerException::what() const throw()
{
	return m_msg;
}

