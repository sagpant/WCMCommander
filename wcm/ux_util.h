#if !defined(UX_UTIL_H) && !defined(_WIN32)
#define UX_UTIL_H

#include <wal.h>

struct MntListNode
{
	std::vector<char> path; //в системной кодировке
	std::vector<char> type;
};

bool UxMntList( wal::ccollect< MntListNode >* pList );

#endif
