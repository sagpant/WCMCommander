#pragma once

#if !defined(_WIN32)

#include <wal.h>

struct MntListNode
{
	std::vector<char> path; //в системной кодировке
	std::vector<char> type;
};

bool UxMntList( wal::ccollect< MntListNode >* pList );

#endif
