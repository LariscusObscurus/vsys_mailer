#ifndef LDAP_H
#define LDAP_H

#define LDAP_DEPRECATED 1

#include <ldap.h>
#include <string>

class Ldap
{
	const char * m_ldapServer;
	LDAP *m_ld;
	LDAPMessage *m_searchResult;
	LDAPMessage *m_entry;
	char * m_attribs[3];
	char *m_dn;
public:
	Ldap(const char * const& ldapServer);
	~Ldap();
	void bind(const char *user, const char *pw);
	bool authenticate(std::string uid, std::string pw, const char* ldapSearchBase);


};

#endif // LDAP_H
