/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#if !defined(_WIN32)

#include <wal.h>

struct MntListNode
{
	std::vector<char> path; //в системной кодировке
	std::vector<char> type;
};

bool UxMntList( wal::ccollect< MntListNode >* pList );
void ExecuteDefaultApplication( const unicode_t* Path );

#endif
