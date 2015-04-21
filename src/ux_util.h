/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#if !defined(_WIN32)

#include <wal.h>

struct MntListNode
{
	std::string path;
	std::string type;
};

bool UxMntList( wal::ccollect< MntListNode >* pList );
void ExecuteDefaultApplication( const unicode_t* Path );

#endif
