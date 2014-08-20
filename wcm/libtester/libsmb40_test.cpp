#include <samba-4.0/libsmbclient.h>

void smbcAuth(const char *srv, const char *shr,  char *wg, int wglen, char *un, int unlen, char *pw, int pwlen)
{
}

int main()
{
	SMBCCTX *smbCTX = smbc_new_context();
	if (!smbCTX) return 1;
	if (!smbc_init_context(smbCTX)) return 1;
	smbc_set_context(smbCTX);
	smbc_setFunctionAuthData(smbCTX, smbcAuth);
	smbc_setOptionUrlEncodeReaddirEntries(smbCTX, 0);
	return 0; 
}

