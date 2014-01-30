#include <libssh2.h>
#include <libssh2_sftp.h>

int main()
{
	if (libssh2_init(0)) return 1;
	return 0; 
}

