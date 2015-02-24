/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#include "charsetdlg.h"
#include "string-util.h"
#include "wcm-config.h"
#include "ltext.h"

#ifdef _WIN32
#  if !defined( NOMINMAX )
#     define NOMINMAX
#  endif
#  include <windows.h>
#endif

#include <unordered_map>

static const char charsetSection[] = "charset";

static ccollect<charset_struct*> csList;

void InitOperCharsets()
{
	csList.clear();
	charset_struct* list[128];
	int count = charset_table.GetList( list, 128 );
	std::unordered_map< std::string, charset_struct* > hash;
	int i;

	for ( i = 0; i < count; i++ )
	{
		hash[list[i]->name] = list[i];
	}

	std::vector< std::string > stringList;

	if ( LoadStringList( charsetSection, stringList ) )
	{
		for ( i = 0; i < ( int )stringList.size(); i++ )
		{
			auto iter = hash.find( stringList[i].data() );

			if ( iter != hash.end( ) ) { csList.append( iter->second ); }
		}
	}



	if ( csList.count() <= 0 )
	{
		const char* lang = sys_locale_lang();

		if ( !strcmp( "ru", lang ) )
		{
#ifdef WIN32
			csList.append( charset_table[CS_UTF8] );
			csList.append( charset_table[CS_WIN1251] );
			csList.append( charset_table[CS_CP866] );
#else
			csList.append( charset_table[CS_UTF8] );
			csList.append( charset_table[CS_WIN1251] );
			csList.append( charset_table[CS_KOI8R] );
			csList.append( charset_table[CS_CP866] );
#endif
		}
	}

	if ( csList.count() <= 0 )
	{
		csList.append( charset_table[CS_UTF8] );
		csList.append( &charsetLatin1 );
	}
}

void SaveOperCharsets()
{
	std::vector< std::string > stringList;

	for ( int i = 0; i < csList.count(); i++ )
	{
		stringList.push_back( std::string( csList[i]->name ) );
	}

	SaveStringList( charsetSection, stringList );
}


int GetFirstOperCharsetId()
{
	return csList.count() > 0 ? csList[0]->id : CS_UTF8;
}

int GetNextOperCharsetId( int id )
{
	if ( csList.count() <= 0 ) { return CS_UTF8; }

	int i;

	for ( i = 0; i < csList.count(); i++ )
		if ( csList[i]->id == id ) { break; }

	return ( i + 1 < csList.count() ) ? csList[i + 1]->id : csList[0]->id;
}


static int  GetOtherCharsetList( charset_struct** list, int size )
{
	charset_struct* tempList[128];
	int retCount = charset_table.GetList( tempList, 128 );
	int i;
	std::unordered_map<int, bool> hash;

	for ( i = 0; i < csList.count(); i++ )
	{
		hash[csList[i]->id] = true;
	}

	int n = 0;

	for ( i = 0; i < retCount; i++ )
	{
		bool Exists = hash.find( tempList[i]->id ) != hash.end();

		if ( !Exists && n < size )
		{
			list[n++] = tempList[i];
		}
	}

	return n;
}


class CharsetListWin: public VListWin
{
	charset_struct** cList;
	int cCount;

	int fontW;
	int fontH;
public:
	CharsetListWin( Win* parent, int H )
		:  VListWin( Win::WT_CHILD, WH_TABFOCUS | WH_CLICKFOCUS, 0, parent, VListWin::SINGLE_SELECT, VListWin::BORDER_3D, nullptr ),
		   cList( 0 ),
		   cCount( 0 )
	{
		wal::GC gc( this );
		gc.Set( GetFont() );
		cpoint ts = gc.GetTextExtents( ABCString );
		fontW = ( ts.x / ABCStringLen );
		fontH = ts.y + 4;

		this->SetItemSize( ( fontH > 16 ? fontH : 16 ) + 1, 100 );

		LSize ls;
		ls.x.maximal = 10000;
		ls.x.ideal = ls.x.minimal = fontW * 57;

		if ( ls.x.minimal < 100 ) { ls.x.minimal = 100; }

		ls.y.maximal = 10000;
		ls.y.ideal = ls.y.minimal = fontH * H;

		if ( ls.y.minimal < 100 ) { ls.y.minimal = 100; }

		SetLSize( ls );
	}

	void SetList( charset_struct** l, int c ) { cList = l; cCount = c; SetCount( c ); }

	int CharsetCount() { return cCount; }
	charset_struct* CurrentCharset() { int n = GetCurrent(); return cList && cCount > 0 && n >= 0 && n < cCount ? cList[n] : 0; }

	virtual void DrawItem( wal::GC& gc, int n, crect rect );

	virtual ~CharsetListWin();
};

void CharsetListWin::DrawItem( wal::GC& gc, int n, crect rect )
{
	if ( cList && n >= 0 && n < cCount )
	{
//		bool frame = false;

		UiCondList ucl;

		if ( ( n % 2 ) == 0 ) { ucl.Set( uiOdd, true ); }

		if ( n == this->GetCurrent() ) { ucl.Set( uiCurrentItem, true ); }

		unsigned bg = UiGetColor( uiBackground, uiItem, &ucl, 0xFFFFFF );
		unsigned textColor = UiGetColor( uiColor, uiItem, &ucl, 0 );
//		unsigned frameColor = UiGetColor( uiFrameColor, uiItem, &ucl, 0 );;
		/*
		      if ( n == this->GetCurrent() )
		      {
		         frame = true;
		      }
		*/
		gc.SetFillColor( bg );
		gc.FillRect( rect );
		gc.Set( GetFont() );

//		int x = 0;
//		const unicode_t* txt = 0;

		gc.SetTextColor( textColor );
		gc.TextOutF( rect.left + 10, rect.top + 2, utf8_to_unicode( cList[n]->name ).data() );
		gc.TextOutF( rect.left + 10 + 15 * fontW, rect.top + 2, utf8_to_unicode( cList[n]->comment ).data() );
	}
	else
	{
		gc.SetFillColor( UiGetColor( uiBackground, uiItem, 0, 0xFFFFFF ) );
		gc.FillRect( rect );
	}
}


CharsetListWin::~CharsetListWin() {};


#define CMD_OTHER 1000
#define CMD_ADD 1001
#define CMD_DEL 1002

class CharsetDlg1: public NCDialog
{
	Layout layout;
	CharsetListWin list;
	Button otherButton;
	Button addButton;
	Button delButton;
public:
	CharsetDlg1( NCDialogParent* parent, int curCharset )
		:  NCDialog( createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Charset" ) ).data(), bListOkCancel ),
		   layout( 10, 10 ),
		   list( this, 7 ),
		   otherButton( 0, this, utf8_to_unicode( _LT( "&Other..." ) ).data(), CMD_OTHER ),
		   addButton( 0, this, utf8_to_unicode( carray_cat<char>( _LT( "&Add..." ), "(Ins)" ).data() ).data(), CMD_ADD ),
		   delButton( 0, this, utf8_to_unicode( carray_cat<char>( _LT( "&Del..." ), "(Del)" ).data() ).data(), CMD_DEL )
	{
//		int i;

		InitOperCharsets();
		list.SetList( ::csList.ptr(), ::csList.count() );
		list.MoveFirst( 0 );

		for ( int i = 0; i < ::csList.count(); i++ )
			if ( ::csList[i]->id == curCharset )
			{
				list.MoveCurrent( i );
				break;
			}

		/*
		   LO
		   L
		   LA
		   LD
		*/
		layout.AddWin( &list, 0, 0, 3, 0 );
		list.Enable();
		list.Show();
		list.SetFocus();
		layout.AddWin( &otherButton, 0, 1 );
		otherButton.Enable();
		otherButton.Show();
		layout.AddWin( &addButton, 2, 1 );
		addButton.Enable();
		addButton.Show();
		layout.AddWin( &delButton, 3, 1 );
		delButton.Enable();
		delButton.Show();

		int mx = 0, my = 0;
		LSize ls;

		otherButton.GetLSize( &ls );

		if ( mx < ls.x.maximal ) { mx = ls.x.maximal; }

		if ( my < ls.y.maximal ) { my = ls.y.maximal; }

		addButton.GetLSize( &ls );

		if ( mx < ls.x.maximal ) { mx = ls.x.maximal; }

		if ( my < ls.y.maximal ) { my = ls.y.maximal; }

		delButton.GetLSize( &ls );

		if ( mx < ls.x.maximal ) { mx = ls.x.maximal; }

		if ( my < ls.y.maximal ) { my = ls.y.maximal; }

		ls.x.minimal = ls.x.maximal = mx;
		ls.y.minimal = ls.y.maximal = my;
		otherButton.SetLSize( ls );
		addButton.SetLSize( ls );
		delButton.SetLSize( ls );

		layout.SetLineGrowth( 1 );

		AddLayout( &layout );
		SetEnterCmd( CMD_OK );
		SetPosition();
	};

	charset_struct* CurrentCharset() { return list.CurrentCharset(); }

	virtual bool Command( int id, int subId, Win* win, void* data );
	virtual bool EventChildKey( Win* child, cevent_key* pEvent );
	virtual ~CharsetDlg1();
	void Del();
	void Add();
};

void CharsetDlg1::Del()
{
	int n = list.GetCurrent();

	if ( n >= 0 && n < csList.count() && csList.count() > 1 &&
	     NCMessageBox( ( NCDialogParent* )Parent(), _LT( "Del" ),
	                   carray_cat<char>( _LT( "Delete element" ), " '", csList[n]->name , "' ?" ).data(), false, bListOkCancel ) == CMD_OK )
	{
		csList.del( n );
		list.SetList( csList.ptr(), csList.count() );
		SaveOperCharsets();
		list.Invalidate();
	}
}


bool CharsetDlg1::Command( int id, int subId, Win* win, void* data )
{
	if ( id == CMD_ITEM_CLICK && win == &list )
	{
		EndModal( CMD_OK );
	}
	else
		switch ( id )
		{
			case CMD_OTHER:
				EndModal( CMD_OTHER );
				return true;

			case CMD_ADD:
				Add();
				return true;

			case CMD_DEL:
				Del();
				return true;
		}

	return NCDialog::Command( id, subId, win, data );
}

bool CharsetDlg1::EventChildKey( Win* child, cevent_key* pEvent )
{
	if ( pEvent->Type() == EV_KEYDOWN )
	{
		if ( pEvent->Key() == VK_DELETE && child == &list )
		{
			Del();
		}
		else if ( pEvent->Key() == VK_INSERT && child == &list )
		{
			Add();
		}
		else if (
		   pEvent->Key() == VK_RETURN && ( child == &otherButton || child == &addButton || child == &delButton )
		)
		{
			return false;
		}
	};

	return NCDialog::EventChildKey( child, pEvent );
}


CharsetDlg1::~CharsetDlg1() {}


class CharsetDialog: public NCDialog
{
	CharsetListWin _list;
public:
	CharsetDialog( NCDialogParent* parent, int curCharset, const char* headerText, charset_struct** cslist, int csCount )
		:  NCDialog( createDialogAsChild, 0, parent, utf8_to_unicode( headerText ).data(), bListOkCancel ),
		   _list( this, 15 )
	{
		_list.SetList( cslist, csCount );

		_list.Enable();
		_list.Show();
		_list.SetFocus();

		_list.MoveFirst( 0 );
		_list.MoveCurrent( 0 );

		for ( int i = 0; i < csCount; i++ )
			if ( cslist[i]->id == curCharset )
			{
				_list.MoveCurrent( i );
				break;
			}

		AddWin( &_list );
		SetEnterCmd( CMD_OK );
		SetPosition();
	};

	charset_struct* CurrentCharset() { return _list.CurrentCharset(); }
	virtual bool Command( int id, int subId, Win* win, void* data );

	virtual ~CharsetDialog();
};

bool CharsetDialog::Command( int id, int subId, Win* win, void* data )
{
	if ( id == CMD_ITEM_CLICK && win == &_list )
	{
		EndModal( CMD_OK );
	}

	return NCDialog::Command( id, subId, win, data );
}


CharsetDialog::~CharsetDialog() {}


bool SelectCharset( NCDialogParent* parent, int* pCharsetId, int currentId )
{
	charset_struct* cslist[128];
	int csCount = charset_table.GetList( cslist, 128 );

	CharsetDialog dlg( parent, currentId, _LT( "other charsets" ), cslist, csCount );
	int ret = dlg.DoModal();

	if ( ret == CMD_OK )
	{
		charset_struct* p =  dlg.CurrentCharset();

		if ( !p ) { return false; }

		if ( pCharsetId ) { *pCharsetId = p->id; }

		return true;
	}

	return false;
}

void CharsetDlg1::Add()
{
	charset_struct* buf[128];
	int bufCount = GetOtherCharsetList( buf, 128 );

	CharsetDialog dlg( ( NCDialogParent* ) Parent(), 0, _LT( "Add charsets" ), buf, bufCount );
	int ret = dlg.DoModal();

	if ( ret == CMD_OK )
	{
		charset_struct* p =  dlg.CurrentCharset();

		if ( !p ) { return; }

		csList.append( p );
		list.SetList( csList.ptr(), csList.count() );
		list.MoveCurrent( csList.count() - 1 );
		SaveOperCharsets();
		return;
	}
}

bool SelectOperCharset( NCDialogParent* parent, int* pCharsetId, int currentId )
{
	int ret = CMD_CANCEL;

	{
		CharsetDlg1 dlg( parent, currentId );
		ret = dlg.DoModal();

		if ( ret == CMD_OK )
		{
			charset_struct* p =  dlg.CurrentCharset();

			if ( !p ) { return false; }

			if ( pCharsetId ) { *pCharsetId = p->id; }

			return true;
		}
	}

	if ( ret == CMD_OTHER )
	{
		charset_struct* cslist[128];
		int csCount = GetOtherCharsetList( cslist, 128 );

		CharsetDialog dlg( parent, currentId, _LT( "other charsets" ), cslist, csCount );
		int ret = dlg.DoModal();

		if ( ret == CMD_OK )
		{
			charset_struct* p =  dlg.CurrentCharset();

			if ( !p ) { return false; }

			if ( pCharsetId ) { *pCharsetId = p->id; }

			return true;
		}
	}

	return false;
}

