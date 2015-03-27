/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "wal/wal_sys_api.h"

/// convert a single UCS-2 character to lowercase
wal::unicode_t UnicodeLC( wal::unicode_t ch );

/// convert a single UCS-2 character to uppercase
wal::unicode_t UnicodeUC( wal::unicode_t ch );

#if defined(__GNUC__) && defined(_WIN32) && !defined(_WIN64)
#include <string>

// MinGW
namespace std
{
	// dummy implementation
	inline ::std::wstring to_wstring( uint64_t i )
	{
		wchar_t Buffer[255];
		Buffer[0] = 0;
		_ui64tow( i, Buffer, 10 );
		return std::wstring( Buffer );
	}
}
#endif // MinGW
