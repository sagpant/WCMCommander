/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include <swl.h>

using namespace wal;

inline void InitExtensionApp() {};

struct AppList: public iIntrusiveCounter
{
	struct Node
	{
		std::vector<unicode_t> name;

		bool terminal;
		std::vector<unicode_t> cmd;
		//or
		clPtr<AppList> sub;

		Node(): terminal( 0 ) {}
	};
	ccollect<Node> list;
	int Count() const { return list.count(); }
};


std::vector<unicode_t> GetOpenCommand( const unicode_t* uri, bool* needTerminal, const unicode_t** pAppName );

clPtr<AppList> GetAppList( const unicode_t* uri );
