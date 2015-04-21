/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#define LIBSSH2_EXIST

#if defined(__GNUC__)
#  undef LIBSSH2_EXIST
#endif

#if defined(_WIN64)
//		Disable LibSSH2 on Win64 builds
#  undef LIBSSH2_EXIST
#endif // _WIN64
