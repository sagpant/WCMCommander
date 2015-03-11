#pragma once

#if defined(_WIN32)
#	define OS_WINDOWS			1
#elif defined(__APPLE__)
#  define OS_OSX				1
#else
#  define OS_LINUX			1
#endif
