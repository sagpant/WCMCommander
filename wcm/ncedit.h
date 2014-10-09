/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "wal.h"
#include "swl.h"
#include "vfs.h"
#include "mfile.h"
#include "shl.h"

using namespace wal;

extern int uiClassEditor;

extern unsigned  UnicodeLC( unsigned ch );

enum NCEditorInfo
{
   CMD_NCEDIT_INFO      = -100,
   CMD_NCEDIT_CURSOR = 0,
   CMD_NCEDIT_CHANGES   = 1
};



struct EditString
{
	enum Flags { FLAG_LF = 1, FLAG_CR = 2 };
	int size, len;
	short shlId;
	unsigned char flags;
	std::vector<char> data;

	EditString();
	EditString( const EditString& a );

	int Len();
	void Set( const char* s, int l );
	void Insert( char* s, int n, int count );
	void Delete( int n, int count );
	void Append( char* s, int count );
	void Clear( int fl );
	void operator = ( const EditString& a );
	char* Get();

	bool CR() const { return ( flags & FLAG_CR ) != 0; }
	bool LF() const { return ( flags & FLAG_LF ) != 0; }

	void SetCR() { flags |= FLAG_CR; }
	void SetLF() { flags |= FLAG_LF; }

	bool Load( MemFile& f )
	{
		Clear( 0 );
		data = f.ReadToChar( '\n', &size, true );
		len = size;
		return ( size > 0 );
	}
};



class EditList
{
	enum { BSIZE = 1024 };

	int count;
	ccollect<std::vector<EditString>, 0x100> data;
public:
	class Pos
	{
		friend class EditList;
		// b - bufferNo
		// p - posInBuffer
		// n - offset from beg of data
		int b, p, n;
	public:
		Pos(): b( 0 ), p( 0 ), n( 0 ) {}
		Pos( int _n ): b( _n / BSIZE ), p( _n % BSIZE ), n( _n ) {}
		void Set( int _n ) {b = _n / BSIZE; p = _n % BSIZE; n = _n;}
		void operator = ( int n ) { Set( n ); }
		void Inc() { n++; if ( p + 1 < BSIZE ) { p++; } else {b++; p = 0;};}
		void Dec() { n--; if ( p ) { p--; } else {b--; p = BSIZE - 1; } }
		bool operator <( const Pos& a ) { return n < a.n; }
		bool operator >( const Pos& a ) { return n > a.n; }
		bool operator <=( const Pos& a ) { return n <= a.n; }
		bool operator >=( const Pos& a ) { return n >= a.n; }
		bool operator <( int i ) { return n < i; }
		bool operator >( int i ) { return n > i; }
		bool operator <=( int i ) { return n <= i; }
		bool operator >=( int i ) { return n >= i; }
		operator int() { return n; }
	};

	EditList(): count( 0 ) { }
	int Count() { return count; }
	void Clear() { data.clear(); count = 0; }

	EditString& Get( const Pos& pos ) { ASSERT( pos.b >= 0 && pos.b < data.count() ); return data[pos.b][pos.p]; }

	void SetSize( int n )
	{
		if ( n == count ) { return; }

		if ( n < count ) { count = n; return ; }

		int cb = ( n + BSIZE - 1 ) / BSIZE;

		if ( cb > data.count() )
			for ( ; data.count() < cb; )
			{
				data.append( std::vector<EditString>( BSIZE ) );
			}

		count = n;
	}

	void Insert( int n, int cnt, int fl )
	{
		if ( count <= 0 ) { return; }

		SetSize( count + cnt );

		for ( Pos i = count - 1, t = count - 1 - cnt; t >= n; i.Dec(), t.Dec() )
		{
			data[i.b][i.p] = data[t.b][t.p];
		}

		{
			for ( Pos i = n, t = n + cnt; i < t; i.Inc() )
			{
				data[i.b][i.p].Clear( fl );
			}
		}
	}

	void Delete( int n, int cnt )
	{
		for ( Pos i = n, t = n + cnt; t < count; i.Inc(), t.Inc() )
		{
			data[i.b][i.p] = data[t.b][t.p];
		}

		SetSize( count - cnt );
	}

	void Append( int n = 1 )
	{
		ASSERT( count + n > 0 );
		SetSize( count + n );
	};


	void Load( MemFile& f )
	{
		Clear();

		Pos pos;
		bool isNL = true;

		EditString str;

		while ( true )
		{
			if ( !str.Load( f ) )
			{
				break;
			}

			isNL = ( str.size > 0 && str.data[str.size - 1] == '\n' );

			if ( isNL )
			{
				str.len--;
				str.SetLF();

				if ( str.len > 0 && str.data[str.len - 1] == '\r' )
				{
					str.len--;
					str.SetCR();
				}

			}

			Append();
			data[pos.b][pos.p] = str;
			pos.Inc();
		}

		if ( isNL )
		{
			Append();

			if ( pos > 0 )
			{
				Pos prevPos = pos;
				prevPos.Dec();
				data[pos.b][pos.p].Clear( data[prevPos.b][prevPos.p].flags );
			}
			else
			{
				data[pos.b][pos.p].Clear( -1 );
			}
		}
	}

	void Save( MemFile& f )
	{
		f.Clear();
		static const char lf = '\n';
		static const char cr = '\r';

		for ( Pos i = 0; i < count; i.Inc() )
		{
			if ( Get( i ).len > 0 )
			{
				f.Append( Get( i ).Get(), Get( i ).Len() );
			}

			if ( i < count - 1 )
			{
				if ( Get( i ).CR() )
				{
					f.Append( &cr, 1 );
				}

				f.Append( &lf, 1 );
			}
		}
	}

	~EditList() { Clear(); }
};

struct EditPoint
{
	int line;
	int pos; //char position in char *
	EditPoint(): line( 0 ), pos( 0 ) {}
	EditPoint( int l, int p ): line( l ), pos( p ) {}
	void Set( int l, int c ) { line = l; pos = c; }

	bool operator < ( const EditPoint& a ) const { return line < a.line || (line == a.line && pos < a.pos); }
	bool operator <= ( const EditPoint& a ) const { return line < a.line || (line == a.line && pos <= a.pos); }
	bool operator != ( const EditPoint& a ) const { return line != a.line || pos != a.pos; }
	bool operator == ( const EditPoint& a ) const { return line == a.line && pos == a.pos; }
};


///////////////////////// Undo

struct UndoRec: public iIntrusiveCounter
{
	enum TYPE { INSTEXT = 1, DELTEXT = 2, ADDLINE = 3, DELLINE = 4, ATTR = 5 };
	char type;

	char prevAttr; //ATTR
	char attr;  //ATTR, ADDLINE, DELLINE

	int line;   //ALL
	int pos; //INSTEXT, DELTEXT

	std::vector<char> data; //INSTEXT, DELTEXT, ADDLINE, DELLINE
	int dataSize;

	UndoRec* prev, *next;

	UndoRec( int t, int l, int p = 0 ): type( t ), line( l ), pos( p ) {}

	void SetData( const char* s, int size )
	{
		dataSize = size;

		if ( size > 0 )
		{
			data.resize( size );
			memcpy( data.data(), s, size );
		}
	}
};

struct UndoBlock: public iIntrusiveCounter
{
	bool editorChanged;
	bool canAggregate;

	EditPoint beginCursor;
	EditPoint beginMarker;
	EditPoint endCursor;
	EditPoint endMarker;
	int count;
	UndoRec* first, *last;

	void SetBeginPos( EditPoint cur, EditPoint marker )
	{
		beginCursor = cur;
		beginMarker = marker;
	}

	void SetEndPos( EditPoint cur, EditPoint marker )
	{
		endCursor = cur;
		endMarker = marker;
	}

	void Append( UndoRec* p )
	{
		p->prev = last;
		p->next = 0;

		if ( last ) { last->next = p; }
		else { first = p; }

		last = p;
		count++;
	};

	void InsText( int line, int pos, const char* s, int size )
	{
		clPtr<UndoRec> p = new UndoRec( UndoRec::INSTEXT, line, pos );
		p->SetData( s, size );
		Append( p.ptr() );
		p.drop();
	}

	void DelText( int line, int pos, const char* s, int size )
	{
		clPtr<UndoRec> p = new UndoRec( UndoRec::DELTEXT, line, pos );
		p->SetData( s, size );
		Append( p.ptr() );
		p.drop();
	}

	void AddLine( int line, char attr, const char* s, int size )
	{
		clPtr<UndoRec> p = new UndoRec( UndoRec::ADDLINE, line );
		p->SetData( s, size );
		p->attr = attr;
		Append( p.ptr() );
		p.drop();
	}

	void DelLine( int line, char attr, const char* s, int size )
	{
		clPtr<UndoRec> p = new UndoRec( UndoRec::DELLINE, line );
		p->SetData( s, size );
		p->attr = attr;
		Append( p.ptr() );
		p.drop();
	}

	void Attr( int line, char prevAttr, char attr )
	{
		clPtr<UndoRec> p = new UndoRec( UndoRec::ATTR, line );
		p->prevAttr = prevAttr;
		p->attr = attr;
		Append( p.ptr() );
		p.drop();
	}


	UndoBlock( bool aggregate, bool changed ): editorChanged( changed ), canAggregate( aggregate ), first( 0 ), last( 0 ), count( 0 ) {}
	~UndoBlock() { for ( UndoRec* p = first; p; ) { UndoRec* t = p->next; delete p; p = t; } }

private:
	UndoBlock(): editorChanged( true ), canAggregate( false ), first( 0 ), last( 0 ), count( 0 ) {}
};



struct UndoList
{
	enum { MAXSIZE = 32 };
	std::vector< clPtr<UndoBlock> > m_Table;
	int count;
	int pos;

	UndoList()
	:m_Table( MAXSIZE ), count( 0 ), pos( 0 )
	{};

	void Append( clPtr<UndoBlock> p )
	{
		while ( count > pos ) { m_Table[--count] = clPtr<UndoBlock>(); }

		if ( count == MAXSIZE )
		{
			for ( int i = 1; i < count; i++ ) { m_Table[i - 1] = m_Table[i]; }

			count--;
			pos = count;
		}

		UndoBlock* x = count > 0 ? m_Table[count - 1].ptr( ) : 0;

		if ( x &&
		     x->canAggregate && p->canAggregate &&
		     x->endCursor == p->beginCursor && x->endMarker == p->beginMarker )
		{
			for ( UndoRec* r = p->first; r; )
			{
				UndoRec* t = r->next;
				x->Append( r );
				r = t;
			}

			p->first = p->last = 0;
			x->endCursor = p->endCursor;
			x->endMarker = p->endMarker;
			return;
		}

		m_Table[count++] = p;
		pos = count;
	}

	int UndoCount() { return pos; }
	int RedoCount() { return count - pos; }

	UndoBlock* GetUndo( ) { return pos > 0 ? m_Table[--pos].ptr( ) : 0; }
	UndoBlock* GetRedo( bool* nextChg )
	{
		if ( pos >= count ) { return 0; }

		if ( nextChg ) { *nextChg = ( pos + 1 >= count || m_Table[pos + 1]->editorChanged ); }

		return pos < count ? m_Table[pos++].ptr( ) : 0;
	}
		
	void Clear() { while ( count > 0 ) { m_Table[--count] = clPtr<UndoBlock>(); } pos = 0; }
};




//////////////////////////

struct EditScreenChar
{
	bool changed;
	unicode_t ch;
	int fColor;
	int bColor;
	EditScreenChar( unicode_t c, int f, int b ) : changed( true ), ch( c ), fColor( f ), bColor( b ) {}
	EditScreenChar(): changed( true ), ch( 0 ), fColor( 0 ), bColor( 0 ) {}
	void SetAll( bool chg, unicode_t c, int f, int b ) { changed = chg; ch = c; fColor = f; bColor = b; }
	void Set( unicode_t c, int f, int b ) { if ( ch != c || fColor != f || bColor != b ) { changed = true; ch = c; fColor = f; bColor = b; }; }
};


class EditScreen
{
	int rows, cols;
	int data_rows, data_cols;
	std::vector< std::vector<EditScreenChar> > data;
public:
	EditPoint prevCursor, cursor;

	EditScreen(): rows( 0 ), cols( 0 ), prevCursor( -1, 0 ), cursor( -1, 0 ), data_rows( 0 ), data_cols( 0 ) {}
	void Alloc( int r, int c )
	{
		if ( r > 0 && c > 0 && ( r > data_rows ||  c > data_cols ) )
		{
			std::vector< std::vector<EditScreenChar> > p( r );

			for ( int i = 0; i < r; i++ )
			{
				p[i].resize( c );// = new EditScreenChar[c];
			}

			data = p;
			data_rows = r;
			data_cols = c;
		}

		rows = r;
		cols = c;
	}

	void Set( int r, int c, int ce, unicode_t ch, int fc, int bc )
	{
		if ( r < 0 || r >= rows ) { return; }

		if ( c < 0 ) { c = 0; }

		if ( ce > cols ) { ce = cols; }

		if ( c >= ce ) { return; }

		for ( EditScreenChar* p = data[r].data() + c; c < ce; c++, p++ ) { p->Set( ch, fc, bc ); }
	}

	void SetCursor( int r, int c ) { cursor.line = r; cursor.pos = c; }

	int Rows() const { return rows; }
	int Cols() const { return cols; }

	EditScreenChar* Line( int r ) { return r >= 0 && r < rows ? data[r].data() : 0; }

	~EditScreen() {}
};

struct sEditorScrollCtx
{
	int m_FirstLine;
	EditPoint m_Point;
};

class EditWin : public Win
{
	Layout _lo;
	ScrollBar vscroll;
	crect editRect;
	EditList text;
	UndoList undoList;
	charset_struct* charset;
	EditPoint marker, cursor;
	cpoint lastMousePoint;
	int recomendedCursorCol;
	int charH, charW;
	int tabSize;
	bool autoIdent;
	int firstLine;
	int colOffset;
	int rows, cols;

	clPtr<SHL::ShlConf> _shlConf;
	SHL::Shl* _shl;
	int _shlLine;

	bool _changed;
	void SetChanged( int minLine );
	unsigned ColorById( int id );
	void RefreshShl( int n );

	bool InMark( const EditPoint& p ) { return (cursor <= p && p < marker) || (marker <= p && p < cursor); }
	void SendChanges() { if ( Parent() ) { Parent()->SendBroadcast( CMD_NCEDIT_INFO, CMD_NCEDIT_CHANGES, this, 0, 2 ); } }

	void CalcScroll();
	int GetColFromPos( int line, int pos );
	int GetPosFromCol( int line, int nCol );

	char* Ident( EditString& str );
	void DrawRow( wal::GC& gc, int n );
	void DrawCursor( wal::GC& gc );

	void CursorToScreen();
	void CursorToScreen_forReplace();

	void CursorLeft( bool mark );
	void CursorHome( bool mark );
	void CursorEnd( bool mark );
	void CursorRight( bool mark );
	void CursorDown( bool mark );
	void CursorUp( bool mark );
	void PageDown( bool mark );
	void PageUp( bool mark );

	crect GetCursorRect( int x, int y ) const;

	bool StepLeft( EditPoint* p, unicode_t* c );
	bool StepRight( EditPoint* p, unicode_t* c );
	void CursorCtrlLeft( bool mark );
	void CursorCtrlRight( bool mark );

	void CtrlPageDown( bool mark );
	void CtrlPageUp( bool mark );

	void SetCursor( cpoint p, bool mark );
	void SelectAll();

	void ToClipboard();


	void FromClipboard();
	void Cut();

	void Del( bool DeleteWord );
	void Backspace( bool DeleteWord );
	void InsChar( unicode_t ch );
	void SetCharset( charset_struct* cs );
	void DeleteLine();
	void Enter();
	bool DelMarked();

	clPtr<FS> _fs;
	FSPath _path;

	EditScreen screen;

	void __RefreshScreenData();
	void __DrawChanges();

	void Refresh() { __RefreshScreenData(); __DrawChanges(); }

	unsigned shlDEF;
	unsigned shlKEYWORD;
	unsigned shlCOMMENT;
	unsigned shlSTRING;
	unsigned shlPRE;
	unsigned shlNUM;
	unsigned shlOPER;
	unsigned shlATTN;
	/*
	unsigned color_text;
	unsigned color_background;
	unsigned color_mark_text;
	unsigned color_mark_background;
	unsigned color_ctrl;
	unsigned color_cursor;
	*/
	int color_text;
	int color_background;
	int color_mark_text;
	int color_mark_background;
	int color_ctrl;
	int color_cursor;

public:
	enum COLOR_ID
	{
	   COLOR_DEF_ID = 0,
	   COLOR_KEYWORD_ID,
	   COLOR_COMMENT_ID,
	   COLOR_STRING_ID,
	   COLOR_PRE_ID,
	   COLOR_NUM_ID,
	   COLOR_OPER_ID,
	   COLOR_ATTN_ID
	};

	EditWin( Win* parent );

	void EnableShl( bool on );

	bool Changed() { return _changed; }
	void ClearChangedStata() { _changed = false; }

	void Load( clPtr<FS> fs, FSPath& path, MemFile& f );

	void Save( MemFile& f );
	void NextCharset();
	void SetCharset( int n );
	void Clear();

	clPtr<FS> GetFS() { return _fs; }
	void GetPath( FSPath& p ) { p = _path; }

	void SetPath( clPtr<FS> fs, FSPath& p );

	EditPoint GetCursorPos() const { return cursor; }
	void SetCursorPos( EditPoint p );

	#pragma region Save/restore the complete position of the editor
	sEditorScrollCtx GetScrollCtx() const;
	void SetScrollCtx( const sEditorScrollCtx& Ctx );
	#pragma endregion

	int GetCursorCol();
	int GetCursorLine() { return cursor.line; }
	int GetLinesCount() { return text.Count(); };
	int32_t GetCursorSymbol();

	int GetCharsetId();
	const char* GetCharsetName() { return charset->name; }

	void GoToLine( int n )
	{
		if ( n >= text.Count() ) { n = text.Count() - 1; }

		if ( n < 0 ) { n = 0; }

		if ( n >= 0 && n < text.Count() )
		{
			cursor.line = marker.line = n;
			cursor.pos = marker.pos = 0;
			CursorToScreen();
			Refresh();
			SendChanges();
		}
	}

	bool Search( const unicode_t* str, bool sens );
	bool Replace( const unicode_t* from, const unicode_t* to, bool sens );

	bool Undo();
	bool Redo();

	virtual void Paint( wal::GC& gc, const crect& paintRect );
	virtual void EventSize( cevent_size* pEvent );
	virtual bool EventKey( cevent_key* pEvent );
	virtual bool EventMouse( cevent_mouse* pEvent );
	virtual void EventTimer( int tid );
	virtual bool Command( int id, int subId, Win* win, void* data );
	virtual bool Broadcast( int id, int subId, Win* win, void* data );
	virtual int UiGetClassId();
	virtual void OnChangeStyles();
	virtual ~EditWin();
};

//////////////////////// EditString Implementation ////////////////////////////////


inline EditString::EditString(): size( 0 ), len( 0 ), shlId( 0 ),
#ifdef _WIN32
	flags( FLAG_CR | FLAG_LF )
#else
	flags( FLAG_LF )
#endif
{}

inline EditString::EditString( const EditString& a )
	:  size( a.size ),
	   len( a.len ),
	   shlId( a.shlId ),
	   flags( a.flags ),
	   data( a.data )
{
}


inline void EditString::Set( const char* s, int l )
{
	if ( l <= 0 )
	{
		return;
	}

	if ( l > size )
	{
		data.resize( l );
		size = l;
	}

	memcpy( data.data(), s, l );
	len = l;
}

inline void EditString::Insert( char* s, int n, int count )
{
	ASSERT( n >= 0 && n <= len );

	if ( len + count > size )
	{
		int newSize = size * 2;

		if ( newSize < len + count )
		{
			newSize = len + count + 1;
		}

		std::vector<char> p( newSize );

		if ( len > 0 )
		{
			memcpy( p.data(), data.data(), len * sizeof( char ) );
		}

		data = p;
		size = newSize;
	}

	if ( n < len )
	{
		memmove( data.data() + n + count, data.data() + n, ( len - n )*sizeof( char ) );
	}

	if ( count > 0 )
	{
		memcpy( data.data() + n, s, count * sizeof( char ) );
	}

	len = len + count;
}

inline void EditString::Delete( int n, int count )
{
	if ( n >= len || count <= 0 )
	{
		return;
	}

	if ( n + count > len )
	{
		count = len - n;
	}

	if ( n + count < len )
	{
		memmove( data.data() + n, data.data() + n + count, sizeof( char ) * ( len - n - count ) );
	}

	len -= count;
}

inline void EditString::Append( char* s, int count )
{
	Insert( s, len, count );
}

inline void EditString::Clear( int fl )
{
	size = len = 0;

	if ( fl >= 0 ) { flags = fl; }
	else
	{
#ifdef _WIN32
		flags = FLAG_CR | FLAG_LF;
#else
		flags = FLAG_LF;
#endif
	}

	data.clear();
}

inline int EditString::Len() { return len; }

inline char* EditString::Get() { return data.data(); }


inline void EditString::operator = ( const EditString& a )
{
	size = a.size;
	len = a.len;
	shlId = a.shlId;
	flags = a.flags;
	data = a.data;
}
