/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include <wal.h>

class StrConfig: public iIntrusiveCounter
{
	wal::cstrhash<std::string > varHash;
public:
	StrConfig();
	bool Load( const char* s );
	std::vector<char> GetConfig();
	void Set( const char* name, const char* value );
	void Set( const char* name, unsigned value );

	const char* GetStrVal( const char* name );
	int GetIntVal( const char* name ); //<0 - not found

	void Clear();
	~StrConfig();
};
