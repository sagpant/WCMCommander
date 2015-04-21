/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#if defined( _WIN32 )
#  include "libconf_win32.h"
#elif defined( __APPLE__ )
#  include "libconf_osx.h"
#else
#  include "libconf_ux.h"
#endif
