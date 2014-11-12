#pragma once

#include <stdint.h>

#define CHECK_RET( cond, ret_val ) if ( !(cond) ) return (ret_val);

#if _MSC_VER >= 1400
#  define Lvsnprintf _vsnprintf_s
#  define Lsnprintf  _snprintf_s
#  define Lsscanf sscanf_s
#  define Lwcsncpy wcsncpy_s
#  define Lstrncpy strncpy_s
#else
#  define Lvsnprintf vsnprintf
#  define Lsnprintf  snprintf
#  define Lsscanf sscanf
#  define Lwcsncpy wcsncpy
#  define Lstrncpy strncpy
#endif
