#ifndef LIBCONF_WIN32_H
#	define LIBCONF_WIN32_H
#	define LIBSSH2_EXIST

#	if defined(_WIN64)
//		Disable LibSSH2 on Win64 builds
#		undef LIBSSH2_EXIST
#	endif // _WIN64

#endif
