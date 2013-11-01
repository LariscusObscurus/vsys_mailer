#include "Ldap.h"
#include "ServerException.h"

Ldap::Ldap(const char * const& ldapServer) :
	m_ldapServer(ldapServer),
	m_ld(nullptr),
	m_searchResult(nullptr),
	m_entry(nullptr),
	m_dn(nullptr)
{
	int rv;
	if((rv =ldap_initialize(&m_ld, ldapServer)) != LDAP_SUCCESS) {
		std::string error("LDAP: ");
		error+= ldap_err2string(rv);
		throw ServerException(error.c_str());
	}

	m_attribs[0] = strdup("uid");
	m_attribs[1] = strdup("cn");
	m_attribs[2] = nullptr;
}

Ldap::~Ldap()
{
	if(m_ld != nullptr) { ldap_unbind(m_ld); }
	if(m_searchResult != nullptr) ldap_msgfree(m_searchResult);
	if(m_dn != nullptr) free(m_dn);
	free(m_attribs[0]);
	free(m_attribs[1]);
}

void Ldap::bind(const char *user, const char *pw)
{
	int rv;
	if((rv = ldap_bind_s(m_ld, user, pw,LDAP_AUTH_SIMPLE)) != LDAP_SUCCESS) {
		std::string error("LDAP: ");
		error+= ldap_err2string(rv);
		throw ServerException(error.c_str());
	}

}

bool Ldap::authenticate(std::string uid, std::string pw, const char* ldapSearchBase)
{
	int rv;
	bool result = false;
	std::string ldapFilter = "(uid=" + uid + ")";

	LDAP *ldAuth;


	if((rv = ldap_search_s(m_ld, ldapSearchBase, LDAP_SCOPE_SUBTREE, ldapFilter.c_str(), m_attribs, 0, &m_searchResult))
		!= LDAP_SUCCESS)
	{
		std::string error("LDAP: ");
		error+= ldap_err2string(rv);
		throw ServerException(error.c_str());
		return false;
	}

	for(m_entry = ldap_first_entry(m_ld, m_searchResult); m_entry!= nullptr; m_entry = ldap_next_entry(m_ld, m_entry)) {
		m_dn = ldap_get_dn(m_ld,m_entry);
		if(m_dn != nullptr) {
			ldap_initialize(&ldAuth,m_ldapServer);
			rv = ldap_bind_s(ldAuth, m_dn, pw.c_str(), LDAP_AUTH_SIMPLE);
			if(rv != 0) {
				result = false;
			} else {
				result= true;
			}
			ldap_unbind(ldAuth);
		}
	}
	return result;

}
