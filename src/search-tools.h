/*
 * Part of WCM Commander
 * https://github.com/corporateshark/WCMCommander
 * wcm@linderdaum.com
 */

#pragma once

#include <wal.h>

using namespace wal;

struct SNode
{
	char a;
	char b;
	SNode(): a( 0 ), b( 0 ) {}
	SNode( char n1, char n2 ): a( n1 ), b( n2 ) {}
	bool Eq( char c ) const { return c == a || ( b && c == b ); }
	bool Eq( const SNode& x ) const { return a == x.a && b == x.b; }
};

struct SBigNode
{
	char a[16];
	char b[16];
	SBigNode() { a[0] = b[0] = 0; }

	int Eq( const char* str ) const;
	bool Eq( const SBigNode& x ) const;
	bool Set( unicode_t c, bool sens, charset_struct* charset );
	int MaxLen();
	int MinLen();
};

class VSearcher: public iIntrusiveCounter
{
	ccollect<SBigNode> sBigList; //1
	ccollect<SNode> sList; //2
	ccollect<char> bytes; //3
	int mode;
	charset_struct* _cs;
public:
	VSearcher(): mode( 0 ), _cs( 0 ) {}
	void Set( unicode_t* uStr, bool sens, charset_struct* charset ); //throw
	char* Search( char* s, char* end, int* fBytes );
	bool Eq( const VSearcher& ) const;
	charset_struct* Charset() { return _cs; }
	int MinLen();
	int MaxLen();
	~VSearcher() {}
};

class MegaSearcher: public iIntrusiveCounter
{
	ccollect< clPtr< VSearcher > > list;
public:
	MegaSearcher() {}
	bool Set( unicode_t* uStr, bool sens, charset_struct* charset = 0 );
	char* Search( char* s, char* end, int* fBytes, charset_struct** retcs );
	int MinLen();
	int MaxLen();
	int Count() const { return list.count(); }
	~MegaSearcher() {}
};
