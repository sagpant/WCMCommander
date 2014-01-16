#if !defined(UX_UTIL_H) && !defined(_WIN32)
#define UX_UTIL_H

#include <wal.h>

struct MntListNode{
	wal::carray<char> path; //в системной кодировке
	wal::carray<char> type;
};

bool UxMntList( wal::ccollect< MntListNode > *pList);

#endif
