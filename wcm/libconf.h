#ifndef LIBCONF_H
#define LIBCONF_H

#if defined( _WIN32 )
#	include "libconf_win32.h"
#elif defined( __APPLE__ )
#	include "libconf_osx.h"
#else
<<<<<<< HEAD
//#include "libconf_ux.h"
=======
#	include "libconf_ux.h"
>>>>>>> dd739240758db209db957155f8ddb2f3aa159d1c
#endif

#endif
