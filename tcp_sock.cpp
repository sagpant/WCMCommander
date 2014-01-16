#include "tcp_sock.h"

#ifdef _WIN32

bool GetHostIp(const char *utf8, unsigned *pIp, int *err)
{
	unsigned long addr = inet_addr(utf8);
	if (addr != INADDR_NONE) {
		if (pIp) *pIp = addr;
		return true;
	};

	struct hostent *p = gethostbyname(utf8);
	if (!p) {
		if (err) *err = WSAGetLastError();
		return false;
	}

	if (p->h_addrtype != AF_INET || !p->h_addr_list[0]) {
		if (err) *err = EINVAL; // :)
		return false;
	}
	
	addr = ((unsigned long*)p->h_addr_list[0])[0];
	if (pIp) *pIp = addr;
	return true;
}


#else 

#include <netdb.h>


bool GetHostIp(const char *utf8, unsigned *pIp, int *err)
{

	in_addr_t addr = inet_addr(utf8);
	if (addr != INADDR_NONE) {
		if (pIp) *pIp = addr;
		return true;
	};
	
	char buf[4096];
	struct hostent ent, *ret;
	int e;

	if (gethostbyname_r(utf8, &ent, buf, sizeof(buf), &ret, &e))
	{
		if (err) *err = e;
		return false;
	}
	
	if (!ret || ret->h_addrtype != AF_INET || !ret->h_addr_list[0]) {
		if (err) *err = EINVAL; // :)
		return false;
	}
	
	addr = ((in_addr_t*)(ret->h_addr_list[0]))[0];
	if (pIp) *pIp = addr;
	return true;
}

#endif
