#ifndef EXT_APP_H
#define EXT_APP_H

#include <swl.h>
using namespace wal;


inline void InitExtensionApp() {};

struct AppList
{
	struct Node
	{
		carray<unicode_t> name;

		bool terminal;
		carray<unicode_t> cmd;
		//or
		cptr<AppList> sub;

		Node(): terminal( 0 ) {}
	};
	ccollect<Node> list;
	int Count() const { return list.count(); }
//	void Print(int lev=0);
};


carray<unicode_t> GetOpenCommand( const unicode_t* uri, bool* needTerminal, const unicode_t** pAppName );

cptr<AppList> GetAppList( const unicode_t* uri );


#endif
