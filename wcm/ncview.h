/*
   Copyright (c) by Valery Goryachev (Wal)
*/

#ifndef NCVIEW_H
#define NCVIEW_H
#include "swl.h"
#include "vfs.h"

using namespace wal;

extern int uiClassViewer;

enum NCViewerInfo
{
   CMD_NCVIEW_INFO      = -101,
   CMD_NCVIEW_CHANGES   = 1
};


struct ViewerSize
{
	int rows;
	int cols;
	bool Eq( int r, int c ) const { return r == rows && c == cols; }
	bool Eq( const ViewerSize& a ) const { return rows == a.rows && cols == a.cols; }
	bool operator == ( const ViewerSize& a ) const { return Eq( a ); }
	ViewerSize(): rows( 0 ), cols( 0 ) {};
};

struct ViewerMode
{
	bool wrap;
	bool hex;
	charset_struct* charset;
	bool Eq( const ViewerMode& a ) const { return wrap == a.wrap && hex == a.hex && charset == a.charset; }
	bool operator == ( const ViewerMode& a ) const { return Eq( a ); }
	ViewerMode(): wrap( false ), hex( false ), charset( charset_table[CS_UTF8] ) {}
};


struct VSData
{
	ViewerMode mode;
	ViewerSize size;

	int dataSize;
	carray<unicode_t> data;
	carray<char> attr;

	void SetDataSize( int n ) { if ( dataSize < n ) { data.alloc( n ); attr.alloc( n ); dataSize = n; } };

	void _CopyData( const VSData& a )
	{
		int n = size.rows * size.cols;
		SetDataSize( n );

		if ( n > 0 )
		{
			memcpy( data.ptr(), a.data.const_ptr(), n * sizeof( unicode_t ) );
			memcpy( attr.ptr(), a.attr.const_ptr(), n * sizeof( char ) );
		}
	}

	VSData(): dataSize( 0 ) {}
	VSData( const VSData& a ): mode( a.mode ), size( a.size ), dataSize( 0 ) { _CopyData( a ); }
	VSData& operator = ( const VSData& a ) { mode = a.mode; size = a.size; _CopyData( a ); return *this;}
	void SetSize( ViewerSize& s ) { SetDataSize( s.rows * s.cols ); size = s; }
	void Clear() { size.rows = size.cols = 0; }
	~VSData() {}
};

struct VMarker
{
	seek_t begin;
	seek_t end;
	VMarker(): begin( 0 ), end( 0 ) {}
	void Set( seek_t a, seek_t b ) { begin = a; end = b; }
	void Clear() { begin = end = 0; }
	bool In( seek_t p ) const { return p >= begin && p < end; }
	bool Empty() const { return begin >= end; }
};


struct VFPos
{
	seek_t size;
	seek_t begin;
	seek_t end;
	int col;
	int maxLine;
	VMarker marker;
	VFPos(): size( 0 ), begin( 0 ), end( 0 ), col( 0 ), maxLine( 0 ) {}
	void Clear() { size = 0; begin = 0; end = 0; col = 0; maxLine = 0; }
};


class ViewerThreadData;

struct ViewerColors
{
	int bg;
	int fg;
	int ctrl;
	int markFg;
	int markBg;
	int lnFg; //line number foreground (in hex mode)
	int hid;
	int loadBg;
	int loadFg;
};


class ViewWin : public Win
{
	Layout _lo;
	ViewerThreadData* threadData;

	ScrollBar vscroll;
	ScrollBar hscroll;

	crect viewRect;
	crect emptyRect;
	int charW, charH;
	charset_struct* charset;
	ViewerColors colors;

	bool CalcSize();

	bool wrap;
	bool hex;

	VSData lastResult;
	VFPos lastPos;

	carray<unicode_t> loadingText;
	bool drawLoading;

	void CalcScroll();
	void SendChanges() { if ( Parent() ) { Parent()->SendBroadcast( CMD_NCVIEW_INFO, CMD_NCVIEW_CHANGES, this, 0, 2 ); } }

public:
	ViewWin( Win* parent );

	void SetFile( FSPtr fsp, FSPath& path, seek_t size );
	void ClearFile();

	virtual void Paint( wal::GC& gc, const crect& paintRect );
	virtual void EventSize( cevent_size* pEvent );
	virtual bool EventKey( cevent_key* pEvent );
	virtual bool EventMouse( cevent_mouse* pEvent );
	virtual bool Command( int id, int subId, Win* win, void* data );
	virtual void EventTimer( int tid );
	virtual int UiGetClassId();
	virtual void ThreadSignal( int id, int data );
	virtual void OnChangeStyles();

	void WrapUnwrap();
	void HexText();

	void NextCharset();
	void SetCharset( int n );
	void SetCharset( charset_struct* cs );
	const char* GetCharsetName() { return charset->name; }
	int GetCharsetId() {return charset->id;}
	int GetPercent();
	int GetCol();

	FSString Uri();
	bool Search( const unicode_t* str, bool sensitive );

	virtual ~ViewWin();
};

#endif
