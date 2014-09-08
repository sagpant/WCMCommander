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

class clEditFileAssociationsWin: public NCVertDialog
{
public:
	clEditFileAssociationsWin( NCDialogParent* parent )
	 : NCVertDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "Edit file associations" ) ).data(), bListOkCancel )
	 , m_Layout( 16, 2 )
	 , m_FileMaskText( 0, this, utf8_to_unicode( _LT( "A file mask or several file masks" ) ).data() )
	 , m_FileMaskEdit( 0, this, 0, 0, 16 )
	 , m_DescriptionText( 0, this, utf8_to_unicode( _LT( "Description of the file association" ) ).data() )
	 , m_DescriptionEdit( 0, this, 0, 0, 16 )
	 , m_ExecuteCommandText( 0, this, utf8_to_unicode( _LT( "Execute command (used for Enter)" ) ).data() )
	 , m_ExecuteCommandEdit( 0, this, 0, 0, 16 )
	 , m_ExecuteCommandSecondaryText( 0, this, utf8_to_unicode( _LT( "Execute command (used for Ctrl+PgDn)" ) ).data() )
	 , m_ExecuteCommandSecondaryEdit( 0, this, 0, 0, 16 )
	 , m_ViewCommandText( 0, this, utf8_to_unicode( _LT( "View command (used for F3)" ) ).data() )
	 , m_ViewCommandEdit( 0, this, 0, 0, 16 )
	 , m_ViewCommandSecondaryText( 0, this, utf8_to_unicode( _LT( "View command (used for Alt+F3)" ) ).data() )
	 , m_ViewCommandSecondaryEdit( 0, this, 0, 0, 16 )
	 , m_EditCommandText( 0, this, utf8_to_unicode( _LT( "Edit command (used for F4)" ) ).data() )
	 , m_EditCommandEdit( 0, this, 0, 0, 16 )
	 , m_EditCommandSecondaryText( 0, this, utf8_to_unicode( _LT( "View command (used for Alf+F4)" ) ).data() )
	 , m_EditCommandSecondaryEdit( 0, this, 0, 0, 16 )
	{
		m_FileMaskEdit.SetText( utf8_to_unicode( "*" ).data(), true );

		m_Layout.AddWinAndEnable( &m_FileMaskText, 0, 0 );
		m_Layout.AddWinAndEnable( &m_FileMaskEdit, 1, 0 );
		m_Layout.AddWinAndEnable( &m_DescriptionText, 2, 0 );
		m_Layout.AddWinAndEnable( &m_DescriptionEdit, 3, 0 );
		m_Layout.AddWinAndEnable( &m_ExecuteCommandText, 4, 0 );
		m_Layout.AddWinAndEnable( &m_ExecuteCommandEdit, 5, 0 );
		m_Layout.AddWinAndEnable( &m_ViewCommandText, 6, 0 );
		m_Layout.AddWinAndEnable( &m_ViewCommandEdit, 7, 0 );
		m_Layout.AddWinAndEnable( &m_ViewCommandSecondaryText, 8, 0 );
		m_Layout.AddWinAndEnable( &m_ViewCommandSecondaryEdit, 9, 0 );
		m_Layout.AddWinAndEnable( &m_EditCommandText, 10, 0 );
		m_Layout.AddWinAndEnable( &m_EditCommandEdit, 11, 0 );
		m_Layout.AddWinAndEnable( &m_EditCommandSecondaryText, 12, 0 );
		m_Layout.AddWinAndEnable( &m_EditCommandSecondaryEdit, 13, 0 );

		AddLayout( &m_Layout );

		order.append( &m_FileMaskEdit );
		order.append( &m_DescriptionEdit );
		order.append( &m_ExecuteCommandEdit );
		order.append( &m_ExecuteCommandSecondaryEdit );
		order.append( &m_ViewCommandEdit );
		order.append( &m_ViewCommandSecondaryEdit );
		order.append( &m_EditCommandEdit );
		order.append( &m_EditCommandSecondaryEdit );

		SetPosition();
	}

	std::vector<unicode_t> GetFileMask() { return m_FileMaskEdit.GetText(); }
	std::vector<unicode_t> GetDescription() { return m_DescriptionEdit.GetText(); }
	std::vector<unicode_t> GetExecuteCommand() { return m_ExecuteCommandEdit.GetText(); }
	std::vector<unicode_t> GetExecuteCommandSecondary() { return m_ExecuteCommandSecondaryEdit.GetText(); }
	std::vector<unicode_t> GetViewCommand() { return m_ViewCommandEdit.GetText(); }
	std::vector<unicode_t> GetViewCommandSecondary() { return m_ViewCommandSecondaryEdit.GetText(); }
	std::vector<unicode_t> GetEditCommand() { return m_EditCommandEdit.GetText(); }
	std::vector<unicode_t> GetEditCommandSecondary() { return m_EditCommandSecondaryEdit.GetText(); }

private:
	Layout m_Layout;

public:
	StaticLine m_FileMaskText;
	EditLine   m_FileMaskEdit;

	StaticLine m_DescriptionText;
	EditLine   m_DescriptionEdit;

	StaticLine m_ExecuteCommandText;
	EditLine   m_ExecuteCommandEdit;

	StaticLine m_ExecuteCommandSecondaryText;
	EditLine   m_ExecuteCommandSecondaryEdit;

	StaticLine m_ViewCommandText;
	EditLine   m_ViewCommandEdit;

	StaticLine m_ViewCommandSecondaryText;
	EditLine   m_ViewCommandSecondaryEdit;

	StaticLine m_EditCommandText;
	EditLine   m_EditCommandEdit;

	StaticLine m_EditCommandSecondaryText;
	EditLine   m_EditCommandSecondaryEdit;
};

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
	clFileAssociationsListWin m_ListWin;
	Button addCurrentButton;
	Button delButton;
	Button editButton;

public:
	clFileAssociationsListWin::Node* retData;

	clFileAssociationsWin( NCDialogParent* parent )
		:  NCDialog( ::createDialogAsChild, 0, parent, utf8_to_unicode( _LT( "File associations" ) ).data(), bListOkCancel ),
		   lo( 10, 10 ),
		   m_ListWin( this ),
		   addCurrentButton( 0, this, utf8_to_unicode( "+ (Ins)" ).data(), CMD_PLUS ),
		   delButton( 0, this, utf8_to_unicode( "- (Del)" ).data(), CMD_MINUS ),
		   editButton( 0, this, utf8_to_unicode( _LT( "Edit" ) ).data(), CMD_EDIT ),
		   retData( 0 )
	{
		m_ListWin.Show();
		m_ListWin.Enable();
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

		lo.AddWin( &m_ListWin, 0, 0, 9, 0 );

		lo.AddWin( &addCurrentButton, 0, 1 );
		lo.AddWin( &delButton, 1, 1 );
		lo.AddWin( &editButton, 2, 1 );
		lo.SetLineGrowth( 9 );

		this->AddLayout( &lo );
		//MaximizeIfChild();

		SetPosition();

		m_ListWin.SetFocus();
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
	clFileAssociationsListWin::Node* node = m_ListWin.GetCurrentData();

	if ( node && node->conf.ptr() && node->name.data() )
	{
		retData = node;
		EndModal( CMD_OK );
	}
}

bool clFileAssociationsWin::Command( int id, int subId, Win* win, void* data )
{
	if ( id == CMD_ITEM_CLICK && win == &m_ListWin )
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
		clFileAssociationsListWin::Node* node = m_ListWin.GetCurrentData();

		if ( !node || !node->name.data() ) { return true; }

		if ( NCMessageBox( ( NCDialogParent* )Parent(), _LT( "Delete item" ),
		                   carray_cat<char>( _LT( "Delete '" ), unicode_to_utf8( node->name.data() ).data() , "' ?" ).data(),
		                   false, bListOkCancel ) == CMD_OK )
		{
			m_ListWin.Del();
		}

		return true;
	}

	if ( id == CMD_EDIT )
	{
		return true;
	}

	if ( id == CMD_PLUS )
	{
		clEditFileAssociationsWin dlg( ( NCDialogParent* )Parent() );
		dlg.SetEnterCmd( 0 );

		if ( dlg.DoModal() == CMD_OK )
		{
//			m_ListWin.Ins( name.data(), cfg );
		}

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

		if ( pEvent->Key() == VK_F4 )
		{
			Command( CMD_EDIT, 0, this, 0 );
			return true;
		}

		if ( pEvent->Key() == VK_RETURN && m_ListWin.InFocus() )
		{
			Selected();
			return true;
		}

		unicode_t c = UnicodeLC( pEvent->Char() );

		if ( c > 32 )
		{
			m_ListWin.Next( c );
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
