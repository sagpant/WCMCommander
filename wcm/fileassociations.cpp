#include <swl.h>
#include "fileassociations.h"
#include "string-util.h"
#include "wcm-config.h"
#include "ltext.h"
#include "strconfig.h"
#include "unicode_lc.h"

static const int CMD_PLUS  = 1000;
static const int CMD_MINUS = 1001;
static const int CMD_EDIT  = 1002;

inline int CmpNoCase( const unicode_t* a, const unicode_t* b )
{
	unicode_t au = 0;
	unicode_t bu = 0;

	for ( ; *a; a++, b++ )
	{
		au = UnicodeLC( *a );
		bu = UnicodeLC( *b );

		if ( au != bu ) { break; }
	};

	return ( *a ? ( *b ? ( au < bu ? -1 : ( au == bu ? 0 : 1 ) ) : 1 ) : ( *b ? -1 : 0 ) );
}


static const char fileAssociationsSection[] = "fileassociations";

class clFileAssociationsListWin: public VListWin
{
public:
	struct Node
	{
		std::vector<unicode_t> name;
		clPtr<StrConfig> conf;
	};

private:
	ccollect<Node> itemList;

	void Load()
	{
		itemList.clear();

		ccollect< std::vector<char> > list;
		LoadStringList( fileAssociationsSection, list );

		for ( int i = 0; i < list.count(); i++ )
		{
			if ( list[i].data() )
			{
				clPtr<StrConfig> cfg = new StrConfig();
				cfg->Load( list[i].data() );
				const char* name = cfg->GetStrVal( "name" );

				if ( name )
				{
					Node node;
					node.conf = cfg;
					node.name = utf8_to_unicode( name );
					itemList.append( node );
				}
			}
		}

		//void SaveStringList(const char *section, ccollect< std::vector<char> > &list);
	}
public:
	clFileAssociationsListWin( Win* parent )
		:  VListWin( Win::WT_CHILD, WH_TABFOCUS | WH_CLICKFOCUS, 0, parent, VListWin::SINGLE_SELECT, VListWin::BORDER_3D, 0 )
	{
		wal::GC gc( this );
		gc.Set( GetFont() );
		cpoint ts = gc.GetTextExtents( ABCString );
		int fontW = ( ts.x / ABCStringLen );
		int fontH = ts.y + 2;

		this->SetItemSize( ( fontH > 16 ? fontH : 16 ) + 1, 100 );

		Load();
		SetCount( itemList.count() );
		SetCurrent( 0 );

		LSize ls;
		ls.x.maximal = 10000;
		ls.x.ideal = 500;
		ls.x.minimal = 300;

		ls.y.maximal = 10000;
		ls.y.ideal = 300;
		ls.y.minimal = 200;

		SetLSize( ls );
	}

	Node* GetCurrentData() { int n = GetCurrent(); if ( n < 0 || n >= itemList.count() ) { return 0; } return &( itemList[n] ); }

	void Save();

	void Ins( const unicode_t* name, clPtr<StrConfig> p )
	{
		if ( !name || !*name ) { return; }

		Node node;
		node.name = new_unicode_str( name );
		node.conf = p;

		itemList.append( node );
		this->SetCount( itemList.count() );
		this->SetCurrent( itemList.count() - 1 );
		Save();
		CalcScroll();
		Invalidate();
	}

	void Next( int ch );

	void Del()
	{
		int n = GetCurrent();

		if ( n < 0 || n >= itemList.count() ) { return; }

		itemList.del( n );
		SetCount( itemList.count() );
		Save();
		CalcScroll();
		Invalidate();
	}

	void Rename( const unicode_t* name )
	{
		int n = GetCurrent();

		if ( !name || n < 0 || n >= itemList.count() ) { return; }

		itemList[n].name = new_unicode_str( name );
		Save();
		CalcScroll();
		Invalidate();
	}

	virtual void DrawItem( wal::GC& gc, int n, crect rect );
	virtual ~clFileAssociationsListWin();
};

inline bool EqFirst( const unicode_t* s, int c )
{
	return s && UnicodeLC( *s ) == c;
}

void clFileAssociationsListWin::Next( int ch )
{
	unicode_t c = UnicodeLC( ch );
	int i;
	int n = 0;
	int count = itemList.count();
	int current = GetCurrent();

	for ( i = current + 1; i < count; i++ )
		if ( EqFirst( itemList[i].name.data(), c ) ) { goto t; }

	for ( i = 0; i <= current && i < count; i++ )
		if ( EqFirst( itemList[i].name.data(), c ) ) { goto t; }

	return;
t:
	;
	MoveCurrent( i );

	Invalidate();
}

void clFileAssociationsListWin::Save()
{
	ccollect< std::vector<char> > list;

	for ( int i = 0; i < itemList.count(); i++ )
		if ( itemList[i].conf.ptr() && itemList[i].name.data() )
		{
			itemList[i].conf->Set( "name", unicode_to_utf8( itemList[i].name.data() ).data() );
			list.append( itemList[i].conf->GetConfig() );
		}

	SaveStringList( fileAssociationsSection, list );
}

static int uiFcColor = GetUiID( "first-char-color" );

void clFileAssociationsListWin::DrawItem( wal::GC& gc, int n, crect rect )
{
	if ( n >= 0 && n < itemList.count() )
	{
		UiCondList ucl;

		if ( ( n % 2 ) == 0 ) { ucl.Set( uiOdd, true ); }

		if ( n == this->GetCurrent() ) { ucl.Set( uiCurrentItem, true ); }

		unsigned bg = UiGetColor( uiBackground, uiItem, &ucl, 0xB0B000 );
		unsigned color = UiGetColor( uiColor, uiItem, &ucl, 0 );
		unsigned fcColor = UiGetColor( uiFcColor, uiItem, &ucl, 0xFFFF );

		/*
		unsigned bg = ::wcmConfig.whiteStyle ? 0xFFFFFF : 0xB0B000;
		unsigned fg =  0;

		unsigned sBg = InFocus() ? 0x800000 : 0x808080;
		unsigned sFg = 0xFFFFFF;
		*/
		gc.SetFillColor( bg );
		gc.FillRect( rect );

		unicode_t* name = itemList[n].name.data();

		if ( name )
		{
			gc.Set( GetFont() );

			gc.SetTextColor( color );
			gc.TextOutF( rect.left + 10, rect.top + 1, name );
			gc.SetTextColor( fcColor );
			gc.TextOutF( rect.left + 10, rect.top + 1, name, 1 );
		}

	}
	else
	{
		gc.SetFillColor( UiGetColor( uiBackground, 0, 0, 0xB0B000 ) );
		gc.FillRect( rect ); //CCC
	}
}

clFileAssociationsListWin::~clFileAssociationsListWin() {};

class clFileAssociationsWin: public NCDialog
{
	Layout lo;
	clFileAssociationsListWin listWin;
	Button addCurrentButton;
	Button delButton;
	Button editButton;

public:
	clFileAssociationsListWin::Node* retData;

	clFileAssociationsWin( NCDialogParent* parent )
		:  NCDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "File associations" ) ).data(), bListOkCancel ),
		   lo( 10, 10 ),
		   listWin( this ),
		   addCurrentButton( 0, this, utf8_to_unicode( "+ (Ins)" ).data(), CMD_PLUS ),
		   delButton( 0, this, utf8_to_unicode( "- (Del)" ).data(), CMD_MINUS ),
		   editButton( 0, this, utf8_to_unicode( _LT( "Edit" ) ).data(), CMD_EDIT ),
		   retData( 0 )
	{
		listWin.Show();
		listWin.Enable();
		addCurrentButton.Show();
		addCurrentButton.Enable();

		delButton.Show();
		delButton.Enable();
		editButton.Show();
		editButton.Enable();

		LSize lsize = addCurrentButton.GetLSize();
		LSize lsize2 = delButton.GetLSize();
		LSize lsize3 = editButton.GetLSize();

		if ( lsize.x.minimal < lsize2.x.minimal ) { lsize.x.minimal = lsize2.x.minimal; }

		if ( lsize.x.minimal < lsize3.x.minimal ) { lsize.x.minimal = lsize3.x.minimal; }

		if ( lsize.x.maximal < lsize.x.minimal ) { lsize.x.maximal = lsize.x.minimal; }

		addCurrentButton.SetLSize( lsize );
		delButton.SetLSize( lsize );
		editButton.SetLSize( lsize );

		lo.AddWin( &listWin, 0, 0, 9, 0 );

		lo.AddWin( &addCurrentButton, 0, 1 );
		lo.AddWin( &delButton, 1, 1 );
		lo.AddWin( &editButton, 2, 1 );
		lo.SetLineGrowth( 9 );

		this->AddLayout( &lo );
		//MaximizeIfChild();

		SetPosition();

		listWin.SetFocus();
		this->SetEnterCmd( CMD_OK );
	}

	virtual bool Command( int id, int subId, Win* win, void* data );

	bool Key( cevent_key* pEvent );
	virtual int UiGetClassId();
	virtual bool EventChildKey( Win* child, cevent_key* pEvent );
	virtual bool EventKey( cevent_key* pEvent );

	void Selected();

	virtual ~clFileAssociationsWin();
};

int uiClassFileAssociations = GetUiID( "Shortcuts" );

int clFileAssociationsWin::UiGetClassId() { return uiClassFileAssociations; }

void clFileAssociationsWin::Selected()
{
	clFileAssociationsListWin::Node* node = listWin.GetCurrentData();

	if ( node && node->conf.ptr() && node->name.data() )
	{
		retData = node;
		EndModal( CMD_OK );
	}
}

bool clFileAssociationsWin::Command( int id, int subId, Win* win, void* data )
{
	if ( id == CMD_ITEM_CLICK && win == &listWin )
	{
		Selected();
		return true;
	}

	if ( id == CMD_OK )
	{
		Selected();
		return true;
	}

	if ( id == CMD_MINUS )
	{
		clFileAssociationsListWin::Node* node = listWin.GetCurrentData();

		if ( !node || !node->name.data() ) { return true; }

		if ( NCMessageBox( ( NCDialogParent* )Parent(), _LT( "Delete item" ),
		                   carray_cat<char>( _LT( "Delete '" ), unicode_to_utf8( node->name.data() ).data() , "' ?" ).data(),
		                   false, bListOkCancel ) == CMD_OK )
		{
		}

		return true;
	}

	if ( id == CMD_EDIT )
	{
		return true;
	}

	if ( id == CMD_PLUS )
	{
		return true;
	}

	return NCDialog::Command( id, subId, win, data );
}

bool clFileAssociationsWin::Key( cevent_key* pEvent )
{
	if ( pEvent->Type() == EV_KEYDOWN )
	{
		if ( pEvent->Key() == VK_INSERT )
		{
			Command( CMD_PLUS, 0, this, 0 );
			return true;
		}

		if ( pEvent->Key() == VK_DELETE )
		{
			Command( CMD_MINUS, 0, this, 0 );
			return true;
		}

		if ( pEvent->Key() == VK_RETURN && listWin.InFocus() )
		{
			Selected();
			return true;
		}

		unicode_t c = UnicodeLC( pEvent->Char() );

		if ( c > 32 )
		{
			listWin.Next( c );
			return true;
		}
	}

	return false;
}

bool clFileAssociationsWin::EventChildKey( Win* child, cevent_key* pEvent )
{
	if ( Key( pEvent ) ) { return true; }

	return NCDialog::EventChildKey( child, pEvent );
}

bool clFileAssociationsWin::EventKey( cevent_key* pEvent )
{
	if ( Key( pEvent ) ) { return true; }

	return NCDialog::EventKey( pEvent );
}

clFileAssociationsWin::~clFileAssociationsWin()
{
}

bool FileAssociationsDlg( NCDialogParent* parent )
{
	clFileAssociationsWin dlg( parent );
	dlg.SetEnterCmd( 0 );

	if ( dlg.DoModal() == CMD_OK && dlg.retData && dlg.retData->conf.ptr() )
	{
	}

	return false;
}
