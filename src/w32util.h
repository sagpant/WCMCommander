/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#if defined(_WIN32)

#if !defined( NOMINMAX )
#  define NOMINMAX
#endif
#include <windows.h>
#include <wal.h>

std::vector<wchar_t> GetAppPath();

class RegKey
{
	HKEY key;
public:
	RegKey(): key( 0 ) {}
	bool Ok() { return key != 0; }
	void Close();
	HKEY Key() { return key; }
	bool Open( HKEY root, const wchar_t* name, REGSAM sec = KEY_READ );
	bool Create( HKEY root, const wchar_t* name, REGSAM sec = KEY_WRITE | KEY_READ );
	std::vector<wchar_t> GetString( const wchar_t* name = 0, const wchar_t* def = 0 );
	std::vector<wchar_t> SubKey( int n );
	~RegKey();
};

void ExecuteDefaultApplication( const unicode_t* Path );

#endif
