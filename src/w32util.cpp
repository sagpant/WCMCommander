/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#ifdef _WIN32
#include "w32util.h"

using namespace wal;

#include "string-util.h"

std::vector<wchar_t> GetAppPath()
{
	wchar_t buffer[MAX_PATH + 4];

	DWORD l = GetModuleFileNameW( NULL, buffer, sizeof( buffer ) / sizeof( wchar_t ) );
	buffer[l] = 0;

	wchar_t* last = 0;
	wchar_t* s = buffer;

	for ( ; *s; s++ ) if ( *s == '\\' ) { last = s; }

	if ( last ) { last[1] = 0; }
	else { buffer[0] = 0; }

	return new_sys_str( buffer );
}


void RegKey::Close() { if ( key ) { RegCloseKey( key ); key = 0; } }

bool RegKey::Open( HKEY root, const wchar_t* name, REGSAM sec )
{
	Close();
	return RegOpenKeyExW( root, name, 0, sec, &key ) == ERROR_SUCCESS;
}

bool RegKey::Create( HKEY root, const wchar_t* name, REGSAM sec )
{
	Close();
	return RegCreateKeyExW( root, name, 0, 0, REG_OPTION_NON_VOLATILE,
	                        sec, 0, &key, 0 ) == ERROR_SUCCESS;
}

std::vector<wchar_t> RegKey::GetString( const wchar_t* name, const wchar_t* def )
{
	std::vector<wchar_t> strValue;

	DWORD dwType, dwSize;
	LONG lResult = RegQueryValueExW( key, name, NULL, &dwType,
	                                 NULL, &dwSize );

	if ( lResult == ERROR_SUCCESS )
	{
		int n = dwSize / sizeof( wchar_t );
		strValue.resize( n + 1 );
		strValue[n] = 0;

		lResult = RegQueryValueExW( key, name, NULL, &dwType, ( LPBYTE )strValue.data(), &dwSize );

		if ( lResult == ERROR_SUCCESS )
		{
			return strValue;
		}
	}

	return new_wchar_str( def );
}

std::vector<wchar_t> RegKey::SubKey( int n )
{
	DWORD size = 0x100;
	std::vector<wchar_t> name( size );
	LONG res = RegEnumKeyExW( key, n, name.data(), &size, 0, 0, 0, 0 );

	if ( res == ERROR_MORE_DATA )
	{
		size++;
		name.resize( size );
		res = RegEnumKeyExW( key, n, name.data(), &size, 0, 0, 0, 0 );
	}

	if ( res == ERROR_SUCCESS ) { return name; }

	return std::vector<wchar_t>();
}



RegKey::~RegKey() { if ( key ) { RegCloseKey( key ); } }


#endif